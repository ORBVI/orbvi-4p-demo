#include "options.hpp"
#include <cassert>

using namespace orbvi::nv12;
int main() {
    Options options;
    const char *basic[]{"orbvi-nv12-demo"};
    assert(parse_options(1, basic, options) == 0);
    assert(options.groups == 10 && options.output == "nv12-capture");
    const char *configured[]{"demo", "--frames", "5", "--output", "session",
                              "--max-skew-ns", "1000000", "--discard"};
    assert(parse_options(8, configured, options) == 0);
    assert(options.groups == 5 && options.output == "session" && options.discard);
    assert(options.max_skew_ns == 1000000);
    const char *zero[]{"demo", "--frames", "0"};
    assert(parse_options(3, zero, options) == 2);
    const char *extra[]{"demo", "unexpected"};
    assert(parse_options(2, extra, options) == 2);
    const char *help[]{"demo", "--help"};
    assert(parse_options(2, help, options) == 1);
}
