#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" {
#include "../../tools/recomp/runtime/recomp.h"
}

namespace mh4u::native_runtime {

// First independent host layer for the existing recompiler ABI. This is not yet a
// 3DS operating system, GPU, audio backend, or playable game runtime.
class HostRuntime {
public:
    explicit HostRuntime(std::uint32_t memory_size = 1024 * 1024);
    ~HostRuntime() = default;

    HostRuntime(const HostRuntime&) = delete;
    HostRuntime& operator=(const HostRuntime&) = delete;

    RCtx& context() noexcept { return ctx_; }
    const RCtx& context() const noexcept { return ctx_; }

    bool read(std::uint32_t address, std::uint32_t size, std::uint32_t& value) const noexcept;
    bool write(std::uint32_t address, std::uint32_t size, std::uint32_t value) noexcept;
    bool map_function(std::uint32_t address, std::uint32_t mode, RFunc fn);
    bool call(std::uint32_t target) noexcept;

    const std::string& last_error() const noexcept { return last_error_; }
    void clear_error() noexcept { last_error_.clear(); }

private:
    static constexpr std::uint32_t kAddressModeBit = 1;
    RCtx ctx_{};
    std::vector<std::uint8_t> memory_;
    std::unordered_map<std::uint32_t, RFunc> functions_;
    std::string last_error_;
};

} // namespace mh4u::native_runtime
