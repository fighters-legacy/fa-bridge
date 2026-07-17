// SPDX-License-Identifier: GPL-3.0-or-later
#include "BridgeConfig.h"

#include "PlatformPaths.h"

#include <fstream>
#include <string>
#include <string_view>

namespace fa {

namespace {

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
        s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.remove_suffix(1);
    return s;
}

// Parses a TOML basic string ("..." with \\ and \" escapes) from a trimmed
// value. Returns nullopt on any malformation.
std::optional<std::string> parseBasicString(std::string_view value) {
    if (value.size() < 2 || value.front() != '"')
        return std::nullopt;
    std::string out;
    for (size_t i = 1; i < value.size(); ++i) {
        const char c = value[i];
        if (c == '\\') {
            if (i + 1 >= value.size())
                return std::nullopt;
            const char next = value[++i];
            if (next == '\\' || next == '"')
                out.push_back(next);
            else
                return std::nullopt; // escape we never write
        } else if (c == '"') {
            return (i == value.size() - 1) ? std::optional<std::string>(out) : std::nullopt;
        } else {
            out.push_back(c);
        }
    }
    return std::nullopt; // unterminated
}

std::string escapeBasicString(std::string_view raw) {
    std::string out;
    out.reserve(raw.size());
    for (char c : raw) {
        if (c == '\\' || c == '"')
            out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

} // namespace

std::filesystem::path BridgeConfig::defaultPath() {
    const auto dir = configDir();
    if (dir.empty())
        return {};
    return dir / "config.toml";
}

std::optional<BridgeConfig> BridgeConfig::load(const std::filesystem::path& file) {
    if (file.empty())
        return std::nullopt;
    std::ifstream in(file);
    if (!in)
        return std::nullopt;

    std::string line;
    while (std::getline(in, line)) {
        const std::string_view stripped = trim(line);
        if (stripped.empty() || stripped.front() == '#' || stripped.front() == '[')
            continue;
        const auto eq = stripped.find('=');
        if (eq == std::string_view::npos)
            continue;
        if (trim(stripped.substr(0, eq)) != "install_dir")
            continue;
        const auto value = parseBasicString(trim(stripped.substr(eq + 1)));
        if (!value || value->empty())
            return std::nullopt;
        return BridgeConfig{std::filesystem::path(*value)};
    }
    return std::nullopt;
}

bool BridgeConfig::save(const std::filesystem::path& file) const {
    if (file.empty() || installDir.empty())
        return false;

    std::error_code ec;
    const auto parent = file.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec)
            return false;
    }

    std::ofstream out(file, std::ios::trunc);
    if (!out)
        return false;
    out << "# fa-bridge configuration. Written by the plugin's first-run flow;\n"
        << "# safe to edit or delete (deleting re-triggers install discovery).\n"
        << "install_dir = \"" << escapeBasicString(installDir.generic_string()) << "\"\n";
    return out.good();
}

} // namespace fa
