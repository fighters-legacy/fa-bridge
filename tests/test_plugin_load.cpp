// SPDX-License-Identifier: GPL-3.0-or-later

// Exit-gate rehearsal: load the built plugin exactly as the engine's
// ModLoader does (same flags, same symbol constant) and exercise the factory
// contract. argv[1] = plugin path, argv[2] = staged manifest.toml path.
#undef NDEBUG

#include "TempDir.h"
#include "content/IContentPack.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using FactoryFn = fl::IContentPack* (*)();

namespace {

FactoryFn loadFactory(const char* pluginPath) {
#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(pluginPath);
    assert(handle != nullptr);
    return reinterpret_cast<FactoryFn>(GetProcAddress(handle, fl::IContentPack::kFactorySymbol));
#else
    void* handle = dlopen(pluginPath, RTLD_LOCAL | RTLD_NOW);
    if (handle == nullptr)
        std::fprintf(stderr, "dlopen: %s\n", dlerror());
    assert(handle != nullptr);
    return reinterpret_cast<FactoryFn>(dlsym(handle, fl::IContentPack::kFactorySymbol));
#endif
    // The handle is intentionally never closed — the engine keeps plugins
    // loaded for process lifetime.
}

// Manifest text with spaces/tabs stripped, so assertions are immune to the
// key/value alignment in manifest.toml.in.
std::string normalizedManifest(const char* path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    std::string normalized;
    for (char c : ss.str()) {
        if (c != ' ' && c != '\t')
            normalized += c;
    }
    return normalized;
}

} // namespace

int main(int argc, char** argv) {
    assert(argc == 3);

    // Hermetic: init() below runs the real discovery chain — never let it read
    // this machine's persisted config, registry, or drives.
    fatest::HermeticEnv hermetic;

    FactoryFn factory = loadFactory(argv[1]);
    assert(factory != nullptr);

    // Two calls must produce two distinct heap instances (never a singleton):
    // the engine wraps each result in unique_ptr and deletes it.
    fl::IContentPack* a = factory();
    fl::IContentPack* b = factory();
    assert(a != nullptr && b != nullptr && a != b);

    assert(std::strcmp(a->id(), "fa-bridge") == 0);
    assert(std::strcmp(a->namespaceId(), "fa") == 0);
    assert(a->priority() == 100);
    assert(a->isNativePlugin());
    assert(a->version() != nullptr && *a->version() != '\0');
    // Must not crash across the plugin boundary; the resulting state depends
    // on FA_INSTALL_DIR in the test environment.
    (void)a->init();

    // The staged manifest must agree with the pack it ships beside.
    const std::string manifest = normalizedManifest(argv[2]);
    assert(!manifest.empty());
    assert(manifest.find("id=\"fa-bridge\"") != std::string::npos);
    assert(manifest.find("namespace=\"fa\"") != std::string::npos);
    assert(manifest.find("priority=100") != std::string::npos);
    assert(manifest.find("engine-api=\"1.0\"") != std::string::npos);
    assert(manifest.find(std::string("version=\"") + a->version() + "\"") != std::string::npos);

    // Deletion through the interface pointer — the unique_ptr path the engine
    // takes at shutdown.
    delete a;
    delete b;

    std::puts("test_plugin_load: OK");
    return 0;
}
