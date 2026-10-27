#include "host_runtime.h"

#include <limits>

namespace mh4u::native_runtime {

HostRuntime::HostRuntime(std::uint32_t memory_size) : memory_(memory_size, 0) {
    ctx_ = RCtx{};
    ctx_.mem = memory_.data();
    ctx_.user = this;
    ctx_.cpsr_misc = 0x10; // ARM user mode, matching the recompiler's initial convention.
}

bool HostRuntime::read(std::uint32_t address, std::uint32_t size,
                       std::uint32_t& value) const noexcept {
    if (size != 1 && size != 2 && size != 4) return false;
    const auto start = static_cast<std::uint64_t>(address);
    const auto end = start + size;
    if (end > memory_.size() || end < start) return false;

    value = 0;
    for (std::uint32_t i = 0; i < size; ++i) {
        value |= static_cast<std::uint32_t>(memory_[address + i]) << (8 * i);
    }
    return true;
}

bool HostRuntime::write(std::uint32_t address, std::uint32_t size,
                        std::uint32_t value) noexcept {
    if (size != 1 && size != 2 && size != 4) return false;
    const auto start = static_cast<std::uint64_t>(address);
    const auto end = start + size;
    if (end > memory_.size() || end < start) return false;

    for (std::uint32_t i = 0; i < size; ++i) {
        memory_[address + i] = static_cast<std::uint8_t>(value >> (8 * i));
    }
    return true;
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

bool HostRuntime::call(std::uint32_t target) noexcept {
    const auto key = target;
    const auto it = functions_.find(key);
    if (it == functions_.end()) {
        last_error_ = "unmapped guest function target";
        return false;
    }
    it->second(&ctx_);
    return true;
}

} // namespace mh4u::native_runtime
