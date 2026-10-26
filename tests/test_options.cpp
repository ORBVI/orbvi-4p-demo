#include <assert.h>
#include "options.hpp"
int main(void) {
    struct options o;
    const char *basic[] = {"demo"};
    assert(parse_options(1, basic, &o) == 0);
    assert(o.mask == 15 && o.frames == 10 && !o.external);
    const char *one[] = {"demo", "--camera", "2", "--frames", "3"};
    assert(parse_options(5, one, &o) == 0 && o.mask == 4 && o.frames == 3);
    const char *bad[] = {"demo", "--frames", "-1"};
    assert(parse_options(3, bad, &o) < 0);
    const char *zero[] = {"demo", "--timeout-ms", "0"};
    assert(parse_options(3, zero, &o) < 0);
    const char *extra[] = {"demo", "unexpected"};
    assert(parse_options(2, extra, &o) < 0);
    const char *mode[] = {"demo", "--mode", "external"};
    assert(parse_options(3, mode, &o) == 0 && o.external);
    const char *invalid[] = {"demo", "--camera", "4"};
    assert(parse_options(3, invalid, &o) < 0);
    const char *trigger[] = {"demo", "--mode", "external", "--fps", "60", "--vts", "1400"};
    assert(parse_options(7, trigger, &o) == 0 && o.fps == 60 && o.vts == 1400);
    const char *bad_fps[] = {"demo", "--mode", "external", "--fps", "40"};
    assert(parse_options(5, bad_fps, &o) < 0);
    const char *bad_vts[] = {"demo", "--mode", "external", "--vts", "1280"};
    assert(parse_options(5, bad_vts, &o) < 0);
    const char *free_vts[] = {"demo", "--vts", "1400"};
    assert(parse_options(3, free_vts, &o) < 0);
    const char *readout[] = {"demo", "--mode", "external", "--trigger-readout"};
    assert(parse_options(4, readout, &o) == 0 && o.trigger_readout);
    const char *free_readout[] = {"demo", "--trigger-readout"};
    assert(parse_options(2, free_readout, &o) < 0);
    assert(parse_options(1, basic, &o) == 0 && !o.trigger_readout);
    return 0;
}
