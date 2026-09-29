#include "hbn_api.h"
#include <dlfcn.h>
#include <stdio.h>

extern "C" hobot_status hbn_vflow_stop(hbn_vflow_handle_t flow)
{
    using stop_fn = hobot_status (*)(hbn_vflow_handle_t);
    auto stop = reinterpret_cast<stop_fn>(dlsym(RTLD_NEXT, "hbn_vflow_stop"));
    if (!stop) return -1;
    auto result = stop(flow);
    fputs("INJECTED: stop flow failure after real cleanup\n", stderr);
    return result ? result : -1;
}
