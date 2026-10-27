#include "code_module.h"

#include <dlfcn.h>

#include <utility>

namespace mh4u::native_runtime {

CodeModule::~CodeModule() {
    unload();
}

bool CodeModule::load(const std::string& path, HostRuntime& runtime) {
    if (loaded()) {
        last_error_ = "a code module is already loaded";
        return false;
    }
    if (path.empty()) {
        last_error_ = "code module path is empty";
        return false;
    }

    dlerror();
    void* candidate = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (candidate == nullptr) {
        const char* error = dlerror();
        last_error_ = error != nullptr ? error : "dlopen failed";
        return false;
    }

    dlerror();
    auto init = reinterpret_cast<RCodeInit>(dlsym(candidate, RC_CODE_INIT_SYMBOL));
    const char* symbol_error = dlerror();
    if (symbol_error != nullptr || init == nullptr) {
        last_error_ = symbol_error != nullptr ? symbol_error : "rc_code_init is missing";
        dlclose(candidate);
        return false;
    }

    RCodeInfo candidate_info{};
    const RHost host = runtime.host_api();
    if (init(&host, &candidate_info) == 0) {
        last_error_ = "rc_code_init rejected the host ABI";
        dlclose(candidate);
        return false;
    }
    if (candidate_info.count != 0 && candidate_info.table == nullptr) {
        last_error_ = "rc_code_init returned a null function table";
        dlclose(candidate);
        return false;
    }

    handle_ = candidate;
    info_ = candidate_info;
    last_error_.clear();
    return true;
}

void CodeModule::unload() noexcept {
    if (handle_ != nullptr) {
        dlclose(handle_);
        handle_ = nullptr;
    }
    info_ = RCodeInfo{};
}

} // namespace mh4u::native_runtime
