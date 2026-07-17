// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Portable environment-variable helpers for tests. On Windows _putenv_s with an
// empty value removes the variable, matching POSIX unsetenv.

#include <cstdlib>

namespace fatest {

inline void setEnv(const char* name, const char* value) {
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

inline void unsetEnv(const char* name) {
#if defined(_WIN32)
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

} // namespace fatest
