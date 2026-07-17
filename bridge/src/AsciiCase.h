// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// ASCII-only case folding. Deliberately not std::tolower: that is UB for
// negative char values and locale-dependent (Turkish-I). FA names are 8.3
// ASCII; anything outside A-Z compares raw.

#include <string>
#include <string_view>

namespace fa {

constexpr char asciiToLower(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}

inline std::string asciiLower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s)
        out.push_back(asciiToLower(c));
    return out;
}

inline bool asciiIEquals(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (asciiToLower(a[i]) != asciiToLower(b[i]))
            return false;
    return true;
}

} // namespace fa
