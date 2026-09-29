HBN_ROOT ?= /opt/x5
CROSS_COMPILE ?= aarch64-linux-gnu-
CXX = $(CROSS_COMPILE)g++
HOST_CXX ?= c++
SDK_INCLUDE = $(HBN_ROOT)/include
SDK_LIB = $(HBN_ROOT)/lib
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Werror -pthread

.PHONY: all test test-sync test-apps build-apps test-wifi test-injection clean
all: dist/orbvi-4p-demo dist/orbvi-verify dist/run.sh dist/sync_time.sh dist/wifi_example.sh

dist/run.sh: scripts/run.sh
	@mkdir -p dist
	install -m 0755 $< $@

dist/sync_time.sh: scripts/sync_time.sh
	@mkdir -p dist
	install -m 0755 $< $@

dist/wifi_example.sh: scripts/wifi_example.sh
	@mkdir -p dist
	install -m 0755 $< $@

SOURCES := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)
VERIFY_SOURCES := $(wildcard tools/*.cpp)
VERIFY_HEADERS := $(wildcard tools/*.hpp)
VERIFY_CORE := tools/validation.cpp tools/runtime.cpp tools/phase.cpp

dist/orbvi-verify: $(VERIFY_SOURCES) $(VERIFY_HEADERS)
	@mkdir -p dist
	$(CXX) $(CXXFLAGS) -Itools $(VERIFY_SOURCES) -o $@

dist/orbvi-4p-demo: $(SOURCES) $(HEADERS)
	@test -r "$(SDK_INCLUDE)/hbn_api.h" || { echo 'Run ./build.sh to prepare public dependencies'; exit 1; }
	@mkdir -p dist
	$(CXX) $(CXXFLAGS) -Isrc -I"$(SDK_INCLUDE)" $(SOURCES) \
		-L"$(SDK_LIB)" \
		-Wl,-rpath-link,"$(SDK_LIB)" \
		-Wl,-rpath,/usr/hobot/lib -Wl,--enable-new-dtags \
		-lcam -lvpf -lhbmem -lalog -lpthread -o $@

build-apps:
	./apps/build.sh

test-apps:
	./apps/test.sh

test: test-sync test-apps test-wifi
	@mkdir -p .build
	$(HOST_CXX) -std=c++17 -Wall -Wextra -Werror -Isrc tests/test_options.cpp src/options.cpp -o .build/test-options
	.build/test-options
	$(HOST_CXX) $(CXXFLAGS) -Isrc tests/test_session.cpp src/capture_session.cpp -o .build/test-session
	.build/test-session
	$(HOST_CXX) $(CXXFLAGS) -Isrc tests/test_file_sink.cpp src/file_sink.cpp -o .build/test-file-sink
	.build/test-file-sink
	@echo 'PASS: options, capture lifecycle and output compatibility'

test-wifi:
	python3 tests/test_wifi_example.py

test-sync:
	@mkdir -p .build
	$(HOST_CXX) $(CXXFLAGS) -Itools tests/test_validation.cpp tools/validation.cpp -o .build/test-validation
	.build/test-validation
	$(HOST_CXX) $(CXXFLAGS) -Itools tests/test_runtime.cpp tools/runtime.cpp tools/validation.cpp -o .build/test-runtime
	.build/test-runtime
	$(HOST_CXX) $(CXXFLAGS) -Itools tests/test_phase.cpp $(VERIFY_CORE) -o .build/test-phase
	.build/test-phase
	$(HOST_CXX) $(CXXFLAGS) -Itools $(VERIFY_SOURCES) -o .build/orbvi-verify
	$(HOST_CXX) $(CXXFLAGS) -Itools tests/test_verify_cli.cpp tools/runtime.cpp tools/validation.cpp -o .build/test-verify-cli
	.build/test-verify-cli .build/orbvi-verify
	@echo 'PASS: C++ validation, runtime, phase accounting and CLI'

# Board-only LD_PRELOAD fault injection; excluded from the release executable.
test-injection:
	@mkdir -p dist
	$(CXX) -std=c++17 -Wall -Wextra -Werror -shared -fPIC -I"$(SDK_INCLUDE)" tests/fail_stop.cpp -ldl -o dist/test-fail-stop.so

clean:
	rm -f dist/orbvi-4p-demo dist/orbvi-verify dist/run.sh dist/sync_time.sh dist/wifi_example.sh .build/test-options .build/test-session .build/test-file-sink .build/test-validation .build/test-runtime .build/test-phase .build/test-verify-cli .build/orbvi-verify
