#include "host_runtime.h"

#include <utility>

namespace mh4u::native_runtime {

HostRuntime::HostRuntime(std::uint32_t memory_size) {
    (void)memory_size;
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

bool HostRuntime::call(std::uint32_t target) noexcept {
    const auto it = functions_.find(target);
    if (it == functions_.end()) {
        last_error_ = "unmapped guest function target";
        return false;
    }
    it->second(&ctx_);
    return true;
}

HostRuntime* HostRuntime::from_context(RCtx* ctx) noexcept {
    return ctx == nullptr ? nullptr : static_cast<HostRuntime*>(ctx->user);
}

void HostRuntime::report_error(std::string message) noexcept {
    last_error_ = std::move(message);
}

RHost HostRuntime::host_api() const noexcept {
    return RHost{RC_HOST_VERSION, static_cast<u32>(sizeof(RCtx)),
        &HostRuntime::callback_call, &HostRuntime::callback_svc, &HostRuntime::callback_trap,
        &HostRuntime::callback_bad_return, &HostRuntime::callback_budget,
        &HostRuntime::callback_slow_read, &HostRuntime::callback_slow_write};
}

void HostRuntime::callback_call(RCtx* ctx, u32 target) {
    auto* runtime = from_context(ctx);
    if (runtime == nullptr || !runtime->call(target)) {
        if (runtime != nullptr) runtime->report_error("guest call callback failed");
    }
}

void HostRuntime::callback_svc(RCtx* ctx, u32 imm, u32 next_pc) {
    (void)next_pc;
    if (auto* runtime = from_context(ctx))
        runtime->report_error("unsupported SVC in native runtime: " + std::to_string(imm));
}

void HostRuntime::callback_trap(RCtx* ctx, u32 pc, u32 kind) {
    if (auto* runtime = from_context(ctx))
        runtime->report_error("guest trap at PC " + std::to_string(pc) +
                              ", kind " + std::to_string(kind));
}

void HostRuntime::callback_bad_return(RCtx* ctx, u32 got, u32 expected) {
    if (auto* runtime = from_context(ctx))
        runtime->report_error("bad guest return: got " + std::to_string(got) +
                              ", expected " + std::to_string(expected));
}

void HostRuntime::callback_budget(RCtx* ctx) {
    if (auto* runtime = from_context(ctx)) runtime->report_error("guest execution budget exhausted");
}

u32 HostRuntime::callback_slow_read(RCtx* ctx, u32 address, u32 size) {
    auto* runtime = from_context(ctx);
    u32 value = 0;
    if (runtime == nullptr || !runtime->read(address, size, value)) {
        if (runtime != nullptr) runtime->report_error("invalid guest memory read");
        return 0;
    }
    return value;
}

void HostRuntime::callback_slow_write(RCtx* ctx, u32 address, u32 value, u32 size) {
    auto* runtime = from_context(ctx);
    if (runtime == nullptr || !runtime->write(address, size, value)) {
        if (runtime != nullptr) runtime->report_error("invalid guest memory write");
    }
}

} // namespace mh4u::native_runtime
