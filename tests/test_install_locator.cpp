// SPDX-License-Identifier: GPL-3.0-or-later

#include "BridgeConfig.h"
#include "FaInstallLocator.h"
#include "TempDir.h"
#include "TestEnv.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>

using fatest::setEnv;
using fatest::TempDir;
using fatest::touchLibFile;

TEST_CASE("isValidInstallDir accepts lib files in any case") {
    TempDir tmp("locator");

    SECTION("uppercase") {
        touchLibFile(tmp.path(), "FA_1.LIB");
        CHECK(fa::FaInstallLocator::isValidInstallDir(tmp.path()));
    }
    SECTION("lowercase") {
        touchLibFile(tmp.path(), "fa_4c.lib");
        CHECK(fa::FaInstallLocator::isValidInstallDir(tmp.path()));
    }
    SECTION("mixed case") {
        touchLibFile(tmp.path(), "Fa_2.LiB");
        CHECK(fa::FaInstallLocator::isValidInstallDir(tmp.path()));
    }
}

TEST_CASE("isValidInstallDir rejects non-installs") {
    TempDir tmp("locator");

    SECTION("empty directory") {
        CHECK_FALSE(fa::FaInstallLocator::isValidInstallDir(tmp.path()));
    }
    SECTION("directory without lib archives") {
        std::ofstream(tmp.path() / "README.TXT") << "not an install";
        std::ofstream(tmp.path() / "FA.EXE").put('\0');
        CHECK_FALSE(fa::FaInstallLocator::isValidInstallDir(tmp.path()));
    }
    SECTION("path is a file") {
        const auto file = tmp.path() / "FA_1.LIB";
        std::ofstream(file).put('\0');
        CHECK_FALSE(fa::FaInstallLocator::isValidInstallDir(file));
    }
    SECTION("missing path") {
        CHECK_FALSE(fa::FaInstallLocator::isValidInstallDir(tmp.path() / "gone"));
    }
}

TEST_CASE("discover honors the chain precedence") {
    fatest::HermeticEnv env;
    TempDir installA("install-a");
    TempDir installB("install-b");
    touchLibFile(installA.path());
    touchLibFile(installB.path());
    const auto configFile = env.configDir() / "config.toml";

    SECTION("valid env var wins over config") {
        setEnv("FA_INSTALL_DIR", installA.path().string().c_str());
        REQUIRE(fa::BridgeConfig{installB.path()}.save(configFile));
        const auto found = fa::FaInstallLocator::discover(configFile, false);
        REQUIRE(found.has_value());
        CHECK(*found == installA.path());
    }

    SECTION("invalid env var falls through to config") {
        setEnv("FA_INSTALL_DIR", (installA.path() / "missing").string().c_str());
        REQUIRE(fa::BridgeConfig{installB.path()}.save(configFile));
        const auto found = fa::FaInstallLocator::discover(configFile, false);
        REQUIRE(found.has_value());
        CHECK(*found == installB.path());
    }

    SECTION("env var pointing at a file falls through") {
        setEnv("FA_INSTALL_DIR", (installA.path() / "FA_1.LIB").string().c_str());
        REQUIRE(fa::BridgeConfig{installB.path()}.save(configFile));
        const auto found = fa::FaInstallLocator::discover(configFile, false);
        REQUIRE(found.has_value());
        CHECK(*found == installB.path());
    }

    SECTION("nothing configured yields nullopt") {
        CHECK_FALSE(fa::FaInstallLocator::discover(configFile, false).has_value());
    }

    SECTION("stale config pointing at a deleted directory yields nullopt") {
        {
            TempDir doomed("doomed");
            touchLibFile(doomed.path());
            REQUIRE(fa::BridgeConfig{doomed.path()}.save(configFile));
        } // TempDir destructor removes the directory
        CHECK_FALSE(fa::FaInstallLocator::discover(configFile, false).has_value());
    }

    SECTION("empty config path skips the config step") {
        CHECK_FALSE(fa::FaInstallLocator::discover({}, false).has_value());
    }
}
