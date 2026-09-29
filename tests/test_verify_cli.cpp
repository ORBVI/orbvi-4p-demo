#include "runtime.hpp"
#include <cassert>
#include <cstdlib>
using namespace orbvi::validation;
int main(int argc,char **argv) {
    assert(argc==2); auto binary=fs::absolute(argv[1]).string();
    auto root=temp_directory("/tmp","orbvi-cli-");
    unsigned index=0;
    auto run=[&](std::vector<std::string> args,int expected) {
        args.insert(args.begin(),binary); Process p;
        auto log=root/std::to_string(index++); p.start(args,log);
        assert(p.wait(2000000000)==expected);
        return read_text(log);
    };
    assert(run({"--help"},0).find("backpressure")!=std::string::npos);
    run({},2); run({"unknown"},2); run({"board"},2);
    run({"sync","/bin/true","--typo"},2);
    run({"shutdown","/bin/true"},2);
    run({"backpressure","--output"},2);
    for(const char *value:{"0","121","-1","abc","","100000000000000000000000"}) {
        assert(setenv("ORBVI_SYNC_SECONDS",value,1)==0);
        run({"sync","/bin/true"},2);
    }
    unsetenv("ORBVI_SYNC_SECONDS");
    // Never launch a real board workflow during host checks, even on an ARM host.
    run({"board",(root/"missing-runner").string()},1);
    fs::remove_all(root);
}
