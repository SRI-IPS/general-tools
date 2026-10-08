#define CATCH_CONFIG_RUNNER

#include <cstring>

#include "catch.hpp"
#include <spdlog/spdlog.h>

int main(int argc, char *const argv[]) {
    spdlog::set_level(spdlog::level::off);
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-r")) {
            spdlog::set_level(spdlog::level::debug);
            break;
        }
    }

    return Catch::Session().run(argc, argv);
}
