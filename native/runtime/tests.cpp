#include "host_runtime.h"
#include "code_module.h"

#include <cassert>
#include <cstdint>
#include <string>

using mh4u::native_runtime::HostRuntime;

static void add_two(RCtx* ctx) {
    ctx->r[0] += 2;
}

int main() {
    HostRuntime runtime;
    std::uint32_t value = 0;

    mh4u::native_runtime::CodeModule module;
    assert(!module.load("", runtime));
    assert(!module.loaded());
    assert(!module.last_error().empty());

    assert(runtime.context().pages != nullptr);
    assert(runtime.map_guest_memory(0, 4096));
    assert(runtime.map_guest_memory(4096, 4096));
    // The recompiler's RC_PAGED fast path consumes this exact page table.
    assert(runtime.context().pages[0] != nullptr);
    assert(runtime.context().pages[1] != nullptr);
    assert(runtime.write(4, 1, 0xAB));
    assert(runtime.read(4, 1, value) && value == 0xAB);
    assert(runtime.write(8, 2, 0xBEEF));
    assert(runtime.read(8, 2, value) && value == 0xBEEF);
    assert(runtime.write(12, 4, 0x12345678));
    assert(runtime.read(12, 4, value) && value == 0x12345678);

    assert(runtime.write(4094, 4, 0xA1B2C3D4));
    assert(runtime.read(4094, 4, value) && value == 0xA1B2C3D4);
    assert(!runtime.read(8192, 4, value));
    assert(!runtime.write(0, 3, 0));
    assert(runtime.map_function(0x1000, 0, add_two));
    runtime.context().r[0] = 40;
    assert(runtime.call(0x1000));
    assert(runtime.context().r[0] == 42);
    assert(!runtime.call(0x2000));
    assert(!runtime.last_error().empty());

    runtime.clear_error();
    const RHost host = runtime.host_api();
    assert(host.version == RC_HOST_VERSION);
    assert(host.ctx_size == sizeof(RCtx));
    assert(host.slow_read(&runtime.context(), 12, 4) == 0x12345678);
    host.slow_write(&runtime.context(), 16, 0xCAFEBABE, 4);
    assert(runtime.read(16, 4, value) && value == 0xCAFEBABE);
    host.svc(&runtime.context(), 0x2A, 0x1004);
    assert(runtime.last_error().find("unsupported SVC") != std::string::npos);

    runtime.clear_error();
    host.trap(&runtime.context(), 0x1010, RC_TRAP_UNSUPPORTED);
    assert(runtime.last_error().find("guest trap") != std::string::npos);

    assert(runtime.unmap_guest_memory(4096, 4096));
    assert(runtime.context().pages[1] == nullptr);
    assert(!runtime.read(4096, 1, value));
    return 0;
}
