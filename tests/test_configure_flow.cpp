// SPDX-License-Identifier: GPL-3.0-or-later

// The configure() first-run flow against a scripted window: dialog loop,
// invalid-selection retry, persistence, and every cancel path.

#include "FaContentPack.h"
#include "FakeWindow.h"
#include "TempDir.h"

#include <catch2/catch_test_macros.hpp>

using fatest::FakeWindow;
using fatest::HermeticEnv;
using fatest::TempDir;
using fatest::touchLibFile;

TEST_CASE("configure returns false with a null window") {
    HermeticEnv env;
    fa::FaContentPack pack;
    CHECK_FALSE(pack.configure(nullptr));
}

TEST_CASE("configure dialog cancel returns false and persists nothing") {
    HermeticEnv env;
    fa::FaContentPack pack;
    FakeWindow window;
    window.folderResults.push_back(std::nullopt);

    CHECK_FALSE(pack.configure(&window));
    CHECK(window.folderDialogCount == 1);
    CHECK(window.messageBoxCalls.empty());
    CHECK_FALSE(std::filesystem::exists(env.configDir() / "config.toml"));
}

TEST_CASE("configure valid selection persists config and init becomes Ready") {
    HermeticEnv env;
    TempDir install("install");
    touchLibFile(install.path());

    {
        fa::FaContentPack pack;
        FakeWindow window;
        window.folderResults.push_back(install.path().string());
        CHECK(pack.configure(&window));
        CHECK(window.folderDialogCount == 1);
        CHECK(window.messageBoxCalls.empty());
        CHECK(std::filesystem::exists(env.configDir() / "config.toml"));
    }

    // A fresh pack (fresh launch) discovers the persisted path with no env var.
    fa::FaContentPack fresh;
    CHECK(fresh.init() == fl::IContentPack::Status::Ready);
}

TEST_CASE("configure invalid selection warns then retry succeeds") {
    HermeticEnv env;
    TempDir install("install");
    TempDir empty("empty");
    touchLibFile(install.path());

    fa::FaContentPack pack;
    FakeWindow window;
    window.folderResults.push_back(empty.path().string());
    window.folderResults.push_back(install.path().string());
    window.messageBoxReturns.push_back(0); // Retry

    CHECK(pack.configure(&window));
    CHECK(window.folderDialogCount == 2);
    REQUIRE(window.messageBoxCalls.size() == 1);
    CHECK(window.messageBoxCalls[0].numButtons == 2);
    CHECK(std::filesystem::exists(env.configDir() / "config.toml"));
}

TEST_CASE("configure invalid selection then cancel or dismiss returns false") {
    HermeticEnv env;
    TempDir empty("empty");

    fa::FaContentPack pack;
    FakeWindow window;
    window.folderResults.push_back(empty.path().string());

    SECTION("cancel button") {
        window.messageBoxReturns.push_back(1);
    }
    SECTION("dismissed with -1") {
        window.messageBoxReturns.push_back(-1);
    }

    CHECK_FALSE(pack.configure(&window));
    CHECK(window.folderDialogCount == 1);
    CHECK(window.messageBoxCalls.size() == 1);
    CHECK_FALSE(std::filesystem::exists(env.configDir() / "config.toml"));
}

TEST_CASE("configure keeps prompting across repeated invalid selections") {
    HermeticEnv env;
    TempDir emptyA("empty-a");
    TempDir emptyB("empty-b");
    TempDir emptyC("empty-c");

    fa::FaContentPack pack;
    FakeWindow window;
    window.folderResults.push_back(emptyA.path().string());
    window.folderResults.push_back(emptyB.path().string());
    window.folderResults.push_back(emptyC.path().string());
    window.folderResults.push_back(std::nullopt); // then the user gives up
    window.messageBoxReturns.assign({0, 0, 0});

    CHECK_FALSE(pack.configure(&window));
    CHECK(window.folderDialogCount == 4);
    CHECK(window.messageBoxCalls.size() == 3);
}
