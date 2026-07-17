// SPDX-License-Identifier: GPL-3.0-or-later

// Link canary for the embedded fx_lib: builds a synthetic EALIB archive in
// memory and round-trips it through the directory parser and extractor. Proves
// fx::lib compiles, links, and behaves on every CI platform.

#include <catch2/catch_test_macros.hpp>
#include <fx/ealib.h>

#include <cstdint>
#include <vector>

TEST_CASE("fx_lib links and round trips a synthetic ealib archive") {
    const std::vector<uint8_t> alpha{0x00, 0x01, 0x02, 0xFF, 0x0A, 0x0D};
    const std::vector<uint8_t> beta{'b', 'e', 't', 'a'};
    const auto lib = fx::ealib_build({{"ALPHA.BIN", alpha}, {"BETA.TXT", beta}});
    REQUIRE_FALSE(lib.empty());

    const auto entries = fx::ealib_read_dir(lib.data(), lib.size());
    REQUIRE(entries.size() == 2);

    const fx::Entry* found = fx::ealib_find(entries, "alpha.bin"); // lookup is case-insensitive
    REQUIRE(found != nullptr);
    CHECK(fx::ealib_extract(lib.data(), lib.size(), *found) == alpha);

    const fx::Entry* betaEntry = fx::ealib_find(entries, "BETA.TXT");
    REQUIRE(betaEntry != nullptr);
    CHECK(fx::ealib_extract(lib.data(), lib.size(), *betaEntry) == beta);

    CHECK(fx::ealib_safe_name("&LOOP.11K") == "_LOOP.11K");
}
