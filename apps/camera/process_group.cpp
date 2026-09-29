#include "process_group.hpp"
#include <algorithm>
#include <cstring>

namespace orbvi::camera {
int initialize_group(SharedFrameGroup &group, unsigned camera_mask, uint64_t max_skew_ns) {
    std::memset(&group, 0, sizeof(group));
    pthread_mutexattr_t mutex_attributes;
    pthread_condattr_t condition_attributes;
    if (pthread_mutexattr_init(&mutex_attributes) || pthread_condattr_init(&condition_attributes)) return -1;
    int status = pthread_mutexattr_setpshared(&mutex_attributes, PTHREAD_PROCESS_SHARED);
    if (!status) status = pthread_condattr_setpshared(&condition_attributes, PTHREAD_PROCESS_SHARED);
    if (!status) status = pthread_mutex_init(&group.mutex, &mutex_attributes);
    if (!status) status = pthread_cond_init(&group.condition, &condition_attributes);
    pthread_mutexattr_destroy(&mutex_attributes); pthread_condattr_destroy(&condition_attributes);
    if (status) return -1;
    group.camera_mask = camera_mask; group.max_skew_ns = max_skew_ns;
    return 0;
}

GroupDecision wait_for_group(SharedFrameGroup &group, unsigned camera_id,
                             uint64_t timestamp_ns, bool new_frame, bool *accepted_leader) {
    if (accepted_leader) *accepted_leader = false;
    if (camera_id >= 4 || !(group.camera_mask & (1u << camera_id))) return GroupDecision::Stop;
    pthread_mutex_lock(&group.mutex);
    if (group.stopped) { pthread_mutex_unlock(&group.mutex); return GroupDecision::Stop; }
    // A faster peer can complete a round between a Hold return and this
    // worker re-entering. Consume that decision rather than waiting forever
    // for a future generation that cannot begin until this worker advances.
    if (!new_frame && group.generation > group.seen_generation[camera_id]) {
        group.seen_generation[camera_id] = group.generation;
        const GroupDecision result = group.decision[camera_id];
        pthread_mutex_unlock(&group.mutex);
        return result;
    }
    const uint64_t observed_generation = group.generation;
    if (new_frame) {
        group.timestamp_ns[camera_id] = timestamp_ns;
        group.ready[camera_id] = 1;
    }
    bool all_ready = true;
    for (unsigned camera = 0; camera < 4; ++camera)
        if ((group.camera_mask & (1u << camera)) && !group.ready[camera]) all_ready = false;
    if (all_ready) {
        unsigned oldest = 0, newest = 0;
        while (!(group.camera_mask & (1u << oldest))) ++oldest;
        newest = oldest;
        for (unsigned camera = oldest + 1; camera < 4; ++camera) {
            if (!(group.camera_mask & (1u << camera))) continue;
            if (group.timestamp_ns[camera] < group.timestamp_ns[oldest]) oldest = camera;
            if (group.timestamp_ns[camera] > group.timestamp_ns[newest]) newest = camera;
        }
        const bool accepted = group.timestamp_ns[newest] - group.timestamp_ns[oldest] <= group.max_skew_ns;
        if (accepted && accepted_leader) *accepted_leader = true;
        for (unsigned camera = 0; camera < 4; ++camera) {
            if (!(group.camera_mask & (1u << camera))) continue;
            group.decision[camera] = accepted ? GroupDecision::Accept : GroupDecision::Hold;
            if (accepted) group.ready[camera] = 0;
        }
        if (!accepted) { group.decision[oldest] = GroupDecision::Drop; group.ready[oldest] = 0; }
        ++group.generation;
        pthread_cond_broadcast(&group.condition);
    } else {
        while (!group.stopped && group.generation == observed_generation)
            pthread_cond_wait(&group.condition, &group.mutex);
    }
    const GroupDecision result = group.stopped ? GroupDecision::Stop : group.decision[camera_id];
    group.seen_generation[camera_id] = group.generation;
    pthread_mutex_unlock(&group.mutex);
    return result;
}

void stop_group(SharedFrameGroup &group) {
    pthread_mutex_lock(&group.mutex); group.stopped = true;
    pthread_cond_broadcast(&group.condition); pthread_mutex_unlock(&group.mutex);
}
void destroy_group(SharedFrameGroup &group) {
    pthread_cond_destroy(&group.condition); pthread_mutex_destroy(&group.mutex);
}
}
