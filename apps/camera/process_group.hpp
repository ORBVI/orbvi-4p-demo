#pragma once

#include <cstdint>
#include <pthread.h>

namespace orbvi::camera {
enum class GroupDecision : unsigned { Hold, Accept, Drop, Stop };
struct SharedFrameGroup {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    uint64_t timestamp_ns[4];
    unsigned char ready[4];
    GroupDecision decision[4];
    uint64_t generation;
    // Each camera has one worker; remember which completed round it observed.
    uint64_t seen_generation[4];
    uint64_t max_skew_ns;
    unsigned camera_mask;
    bool stopped;
};

int initialize_group(SharedFrameGroup &group, unsigned camera_mask, uint64_t max_skew_ns);
GroupDecision wait_for_group(SharedFrameGroup &group, unsigned camera_id,
                             uint64_t timestamp_ns, bool new_frame, bool *accepted_leader = nullptr);
void stop_group(SharedFrameGroup &group);
void destroy_group(SharedFrameGroup &group);
}
