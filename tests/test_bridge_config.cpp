// SPDX-License-Identifier: GPL-3.0-or-later

#include "BridgeConfig.h"
#include "TempDir.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string>

#if !defined(_WIN32)
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace {

void writeFile(const std::filesystem::path& file, const std::string& contents) {
    std::ofstream out(file, std::ios::binary);
    out << contents;
}

} // namespace

TEST_CASE("BridgeConfig save then load round trips the install dir") {
    fatest::TempDir tmp("config");
    const auto file = tmp.path() / "config.toml";

    SECTION("plain path") {
        fa::BridgeConfig config{tmp.path() / "install"};
        REQUIRE(config.save(file));
        const auto loaded = fa::BridgeConfig::load(file);
        REQUIRE(loaded.has_value());
        CHECK(loaded->installDir == std::filesystem::path((tmp.path() / "install").generic_string()));
    }

    SECTION("path with spaces") {
        fa::BridgeConfig config{tmp.path() / "JANES" / "Fighters Anthology"};
        REQUIRE(config.save(file));
        const auto loaded = fa::BridgeConfig::load(file);
        REQUIRE(loaded.has_value());
        CHECK(loaded->installDir.filename() == "Fighters Anthology");
    }
}

TEST_CASE("BridgeConfig load returns nullopt for a missing file") {
    fatest::TempDir tmp("config");
    CHECK_FALSE(fa::BridgeConfig::load(tmp.path() / "does-not-exist.toml").has_value());
    CHECK_FALSE(fa::BridgeConfig::load({}).has_value());
}

TEST_CASE("BridgeConfig load returns nullopt for corrupt content") {
    fatest::TempDir tmp("config");
    const auto file = tmp.path() / "config.toml";

    SECTION("binary garbage") {
        writeFile(file, std::string("\x01\x02\xff\xfe garbage \x00!", 15));
        CHECK_FALSE(fa::BridgeConfig::load(file).has_value());
    }
    SECTION("unterminated quote") {
        writeFile(file, "install_dir = \"/half/open\n");
        CHECK_FALSE(fa::BridgeConfig::load(file).has_value());
    }
    SECTION("bare unquoted value") {
        writeFile(file, "install_dir = /no/quotes\n");
        CHECK_FALSE(fa::BridgeConfig::load(file).has_value());
    }
    SECTION("empty value") {
        writeFile(file, "install_dir = \"\"\n");
        CHECK_FALSE(fa::BridgeConfig::load(file).has_value());
    }
    SECTION("unknown escape") {
        writeFile(file, "install_dir = \"C:\\Users\\fa\"\n"); // \U is not an escape we write
        CHECK_FALSE(fa::BridgeConfig::load(file).has_value());
    }
}

TEST_CASE("BridgeConfig load tolerates comments unknown keys and CRLF") {
    fatest::TempDir tmp("config");
    const auto file = tmp.path() / "config.toml";
    writeFile(file, "# a comment\r\n"
                    "[mod]\r\n"
                    "future_key = 42\r\n"
                    "install_dir = \"/opt/fa\"\r\n"
                    "trailing = \"ignored\"\r\n");
    const auto loaded = fa::BridgeConfig::load(file);
    REQUIRE(loaded.has_value());
    CHECK(loaded->installDir == std::filesystem::path("/opt/fa"));
}

#if !defined(_WIN32)
TEST_CASE("BridgeConfig escapes quotes and backslashes in values") {
    fatest::TempDir tmp("config");
    const auto file = tmp.path() / "config.toml";
    // Legal (if bizarre) POSIX file names; proves the escape/unescape branches.
    const std::string weird = R"(/tmp/we"ird\dir)";
    fa::BridgeConfig config{std::filesystem::path(weird)};
    REQUIRE(config.save(file));
    const auto loaded = fa::BridgeConfig::load(file);
    REQUIRE(loaded.has_value());
    CHECK(loaded->installDir == std::filesystem::path(weird));
}
#endif

TEST_CASE("BridgeConfig save creates missing parent directories") {
    fatest::TempDir tmp("config");
    const auto file = tmp.path() / "nested" / "deeper" / "config.toml";
    fa::BridgeConfig config{tmp.path()};
    REQUIRE(config.save(file));
    CHECK(std::filesystem::exists(file));
}

TEST_CASE("BridgeConfig save reports failure cleanly") {
    fatest::TempDir tmp("config");

    SECTION("empty target path") {
        fa::BridgeConfig config{tmp.path()};
        CHECK_FALSE(config.save({}));
    }
    SECTION("empty install dir") {
        fa::BridgeConfig config{};
        CHECK_FALSE(config.save(tmp.path() / "config.toml"));
    }
#if !defined(_WIN32)
    SECTION("unwritable directory") {
        if (::geteuid() == 0)
            SKIP("running as root: permission bits do not apply");
        const auto locked = tmp.path() / "locked";
        std::filesystem::create_directories(locked);
        ::chmod(locked.c_str(), 0500);
        fa::BridgeConfig config{tmp.path()};
        CHECK_FALSE(config.save(locked / "config.toml"));
        ::chmod(locked.c_str(), 0700); // so TempDir cleanup succeeds
    }
#endif
}
