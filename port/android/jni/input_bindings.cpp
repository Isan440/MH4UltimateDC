// Android-side key-name comparison used by config.cpp. The full SDL input implementation
// belongs to the desktop frontend; the standalone Android app only needs stable identifiers
// here to detect duplicate bindings while loading its shared configuration.
#include "input.h"

#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

namespace App::Input {
namespace {
std::string Normalize(const std::string& value) {
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    std::string result = value.substr(first, last - first);
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

int StableCode(const std::string& name) {
    // FNV-1a: equal key names always compare equal without depending on SDL or platform keycodes.
    std::uint32_t hash = 2166136261u;
    for (const unsigned char c : name) {
        hash = (hash ^ c) * 16777619u;
    }
    // Keep the value positive and reserve zero for an empty/unknown key.
    return static_cast<int>((hash & 0x7fffffffu) | 1u);
}
} // namespace

std::vector<int> KeyCodes(const std::string& spec) {
    std::vector<int> codes;
    std::size_t start = 0;
    while (start <= spec.size()) {
        const std::size_t end = spec.find('|', start);
        const std::string name = Normalize(spec.substr(
            start, end == std::string::npos ? std::string::npos : end - start));
        if (!name.empty()) {
            codes.push_back(StableCode(name));
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return codes;
}
} // namespace App::Input
