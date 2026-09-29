#ifndef ORBVI_OPTIONS_H
#define ORBVI_OPTIONS_H
struct options {
    unsigned mask = 15, frames = 10, timeout_ms = 3000;
    bool external = false, discard = false;
    const char *output = "capture";
};
int parse_options(int argc, const char *const *argv, struct options *o);
void print_usage(void);
#endif
