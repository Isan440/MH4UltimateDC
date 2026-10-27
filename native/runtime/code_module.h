#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "host_runtime.h"

namespace mh4u::native_runtime {

// Owns a recompiled-code shared library and registers its RCodeInfo table.
// The module must outlive any calls through its function table.
class CodeModule {
public:
    CodeModule() = default;
    ~CodeModule();

    CodeModule(const CodeModule&) = delete;
    CodeModule& operator=(const CodeModule&) = delete;

    bool load(const std::string& path, HostRuntime& runtime);
    void unload() noexcept;
    bool loaded() const noexcept { return handle_ != nullptr; }
    const RCodeInfo& info() const noexcept { return info_; }
    const std::string& last_error() const noexcept { return last_error_; }

private:
    void* handle_ = nullptr;
    HostRuntime* runtime_ = nullptr;
    std::vector<std::uint32_t> registered_keys_;
    RCodeInfo info_{};
    std::string last_error_;
};

} // namespace mh4u::native_runtime
