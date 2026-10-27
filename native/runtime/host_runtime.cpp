#include "host_runtime.h"

#include <limits>

namespace mh4u::native_runtime {

HostRuntime::HostRuntime(std::uint32_t memory_size) {\n    (void)memory_size;
    ctx_ = RCtx{};
    ctx_.mem = nullptr;\n    ctx_.pages = memory_.page_table();
    ctx_.user = this;
    ctx_.cpsr_misc = 0x10; // ARM user mode, matching the recompiler's initial convention.
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
