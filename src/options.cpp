#include "options.hpp"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int number(const char *s, unsigned min, unsigned max, unsigned *out)
{
    char *end;
    unsigned long n;
    if (!s[0] || s[0] == '-' || s[0] == '+') return -1;
    errno = 0;
    n = strtoul(s, &end, 10);
    if (errno || *end || n < min || n > max) return -1;
    *out = (unsigned)n;
    return 0;
}

int parse_options(int argc, const char *const *argv, struct options *o)
{
    *o = options{};
    for (int i=1; i<argc; i++) {
        const char *arg=argv[i];
        unsigned camera;
        if (!strcmp(arg,"--help") || !strcmp(arg,"-h")) return 1;
        if (!strcmp(arg,"--discard")) { o->discard=1; continue; }
        if (++i == argc) return -1;
        const char *value=argv[i];
        if (!strcmp(arg,"--camera")) {
            if (!strcmp(value,"all")) o->mask=15;
            else if (number(value,0,3,&camera)) return -1;
            else o->mask=1u<<camera;
        } else if (!strcmp(arg,"--frames")) {
            if (number(value,1,1000000,&o->frames)) return -1;
        } else if (!strcmp(arg,"--timeout-ms")) {
            if (number(value,1,60000,&o->timeout_ms)) return -1;
        } else if (!strcmp(arg,"--output")) {
            if (!value[0]) return -1;
            o->output=value;
        } else if (!strcmp(arg,"--mode")) {
            if (!strcmp(value,"free")) o->external=0;
            else if (!strcmp(value,"external")) o->external=1;
            else return -1;
        } else return -1;
    }
    return 0;
}

void print_usage(void)
{
    puts("ORBVI 4P Demo - SC132GS RAW capture\n"
         "Usage: orbvi-4p-demo [options]\n"
         "  --camera all|0|1|2|3   CSI port (default: all)\n"
         "  --frames N             Frames per camera (default: 10)\n"
         "  --output DIR           New output directory (default: capture)\n"
         "  --mode free|external   Free running or external slave mode\n"
         "  --timeout-ms N         Maximum time without a frame (default: 3000)\n"
         "  --discard              Write CSV only, discard pixel data\n"
         "  --help                 Show help\n"
         "External mode requires a running external trigger source before startup.");
}
