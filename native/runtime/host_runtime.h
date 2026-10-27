#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../memory/guest_memory.h"

extern "C" {
#include "../../tools/recomp/runtime/recomp.h"
}

namespace mh4u::native_runtime {

class HostRuntime {
public:
    HostRuntime();
    ~HostRuntime() = default;
    HostRuntime(const HostRuntime&) = delete;
    HostRuntime& operator=(const HostRuntime&) = delete;

    RCtx& context() noexcept { return ctx_; }
    const RCtx& context() const noexcept { return ctx_; }

    bool map_guest_memory(std::uint32_t address, std::uint32_t size);
    bool unmap_guest_memory(std::uint32_t address, std::uint32_t size);
    bool read(std::uint32_t address, std::uint32_t size, std::uint32_t& value) const noexcept;
    bool write(std::uint32_t address, std::uint32_t size, std::uint32_t value) noexcept;
    bool map_function(std::uint32_t address, std::uint32_t mode, RFunc fn);
    bool call(std::uint32_t target) noexcept;
    RHost host_api() const noexcept;

    const std::string& last_error() const noexcept { return last_error_; }
    void clear_error() noexcept { last_error_.clear(); }
    void report_error(std::string message) noexcept;

private:
    static HostRuntime* from_context(RCtx* ctx) noexcept;
    static void callback_call(RCtx* ctx, u32 target);
    static void callback_svc(RCtx* ctx, u32 imm, u32 next_pc);
    static void callback_trap(RCtx* ctx, u32 pc, u32 kind);
    static void callback_bad_return(RCtx* ctx, u32 got, u32 expected);
    static void callback_budget(RCtx* ctx);
    static u32 callback_slow_read(RCtx* ctx, u32 address, u32 size);
    static void callback_slow_write(RCtx* ctx, u32 address, u32 value, u32 size);

    static constexpr std::uint32_t kAddressModeBit = 1;
    RCtx ctx_{};
    GuestMemory memory_;
    std::unordered_map<std::uint32_t, RFunc> functions_;
    std::string last_error_;
};

} // namespace mh4u::native_runtime
