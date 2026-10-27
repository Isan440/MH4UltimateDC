#include "host_runtime.h"

#include <algorithm>
#include <utility>

namespace mh4u::native_runtime {

HostRuntime::HostRuntime() {
    ctx_ = RCtx{};
    ctx_.mem = nullptr;
    ctx_.pages = memory_.page_table();
    ctx_.user = this;
    ctx_.cpsr_misc = 0x10;
}

bool HostRuntime::map_guest_memory(std::uint32_t address, std::uint32_t size) {
    return memory_.map(address, size);
}

bool HostRuntime::unmap_guest_memory(std::uint32_t address, std::uint32_t size) {
    return memory_.unmap(address, size);
}

bool HostRuntime::read(std::uint32_t address, std::uint32_t size,
                       std::uint32_t& value) const noexcept {
    return memory_.read(address, size, value);
}

bool HostRuntime::write(std::uint32_t address, std::uint32_t size,
                        std::uint32_t value) noexcept {
    return memory_.write(address, size, value);
}

bool HostRuntime::map_function(std::uint32_t address, std::uint32_t mode, RFunc fn) {
    if (fn == nullptr || mode > 1) {
        last_error_ = "invalid function mapping";
        return false;
    }
    const auto key = (address & ~kAddressModeBit) | mode;
    const auto inserted = functions_.emplace(key, fn).second;
    if (!inserted) last_error_ = "function address already mapped";
    return inserted;
}

bool HostRuntime::register_code_table(const RFuncEntry* table, std::uint32_t count,
                                      std::vector<std::uint32_t>& registered_keys) {
    registered_keys.clear();
    if (count != 0 && table == nullptr) {
        last_error_ = "code module function table is null";
        return false;
    }

    std::vector<std::uint32_t> keys;
    keys.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        if (table[i].fn == nullptr || table[i].mode > 1) {
            last_error_ = "code module contains an invalid function entry";
            return false;
        }
        keys.push_back((table[i].addr & ~kAddressModeBit) | table[i].mode);
    }

    // Sort once so duplicate validation scales as O(n log n), not O(n^2).
    std::sort(keys.begin(), keys.end());
    if (std::adjacent_find(keys.begin(), keys.end()) != keys.end()) {
        last_error_ = "code module contains duplicate function addresses";
        return false;
    }
    for (const auto key : keys) {
        if (functions_.find(key) != functions_.end()) {
            last_error_ = "code module function address already mapped";
            return false;
        }
    }

    // Insert only after all entries validate. Roll back if allocation/insertion fails.
    std::vector<std::uint32_t> inserted_keys;
    inserted_keys.reserve(count);
    try {
        for (std::uint32_t i = 0; i < count; ++i) {
            const auto key = (table[i].addr & ~kAddressModeBit) | table[i].mode;
            const auto inserted = functions_.emplace(key, table[i].fn).second;
            if (!inserted) {
                for (const auto inserted_key : inserted_keys) functions_.erase(inserted_key);
                last_error_ = "code module function address already mapped";
                return false;
            }
            inserted_keys.push_back(key);
        }
    } catch (...) {
        for (const auto inserted_key : inserted_keys) functions_.erase(inserted_key);
        last_error_ = "failed to allocate code module function registrations";
        return false;
    }

    registered_keys = std::move(inserted_keys);
    last_error_.clear();
    return true;
}

void HostRuntime::unregister_code_table(const std::vector<std::uint32_t>& keys) noexcept {
    for (const auto key : keys) functions_.erase(key);
}

bool HostRuntime::call(std::uint32_t target) noexcept {
    const auto it = functions_.find(target);
    if (it == functions_.end()) {
        last_error_ = "unmapped guest function target";
        return false;
    }
    it->second(&ctx_);
    return true;
}

} // namespace mh4u::native_runtime
