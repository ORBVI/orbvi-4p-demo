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
    return 0;
}
