#include "host_runtime.h"

namespace mh4u::native_runtime {

HostRuntime::HostRuntime(std::uint32_t memory_size) {
    (void)memory_size; // Kept temporarily for source compatibility with the initial test harness.
    ctx_ = RCtx{};
    ctx_.mem = nullptr;
    ctx_.pages = memory_.page_table();
    ctx_.user = this;
    ctx_.cpsr_misc = 0x10;
}

bool HostRuntime::map_guest_memory(std::uint32_t address, std::uint32_t size) {\n    return memory_.map(address, size);\n}\n\nbool HostRuntime::unmap_guest_memory(std::uint32_t address, std::uint32_t size) {\n    return memory_.unmap(address, size);\n}\n\nbool HostRuntime::read(std::uint32_t address, std::uint32_t size,
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
    const auto it = functions_.find(target);
    if (it == functions_.end()) {
        last_error_ = "unmapped guest function target";
        return false;
    }
    it->second(&ctx_);
    return true;
}

} // namespace mh4u::native_runtime
