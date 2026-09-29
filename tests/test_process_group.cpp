#include "process_group.hpp"
#include <array>
#include <cassert>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>

int main() {
    orbvi::camera::SharedFrameGroup group;
    assert(orbvi::camera::initialize_group(group, 0xf, 1000) == 0);
    std::array<unsigned, 4> accepted{}, dropped{};
    unsigned leaders = 0; std::mutex leader_mutex;
    std::array<std::thread, 4> workers;
    for (unsigned camera = 0; camera < 4; ++camera) {
        workers[camera] = std::thread([&, camera] {
            uint64_t timestamp = camera ? 2000000 + camera * 100 : 1000000;
            bool fresh = true;
            while (!accepted[camera]) {
                bool leader = false;
                const auto decision = orbvi::camera::wait_for_group(group, camera, timestamp, fresh, &leader);
                if (leader) { std::lock_guard<std::mutex> lock(leader_mutex); ++leaders; }
                fresh = false;
                if (decision == orbvi::camera::GroupDecision::Drop) {
                    ++dropped[camera]; timestamp = 2000000; fresh = true;
                } else if (decision == orbvi::camera::GroupDecision::Accept) ++accepted[camera];
            }
        });
    }
    for (auto &worker : workers) worker.join();
    assert(dropped[0] == 1);
    assert(leaders == 1);
    for (unsigned camera = 0; camera < 4; ++camera) assert(accepted[camera] == 1);
    // Simulate a Hold worker delayed until a faster peer has completed the
    // next round; it must consume the already-published Accept, not sleep.
    group.seen_generation[1] = group.generation - 1;
    std::promise<orbvi::camera::GroupDecision> decision_promise;
    auto decision_future = decision_promise.get_future();
    std::thread late_worker([&] {
        decision_promise.set_value(orbvi::camera::wait_for_group(group, 1, 0, false));
    });
    const bool caught_up = decision_future.wait_for(std::chrono::milliseconds(500)) ==
                           std::future_status::ready;
    if (!caught_up) orbvi::camera::stop_group(group); // Release worker before failing.
    late_worker.join();
    assert(caught_up && decision_future.get() == orbvi::camera::GroupDecision::Accept);
    orbvi::camera::destroy_group(group);
}
