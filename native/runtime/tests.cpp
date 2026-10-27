#include "host_runtime.h"

#include <cassert>
#include <cstdint>

using mh4u::native_runtime::HostRuntime;

static void add_two(RCtx* ctx) {
    ctx->r[0] += 2;
}

int main() {
    HostRuntime runtime(256);
    std::uint32_t value = 0;

    assert(runtime.write(4, 1, 0xAB));
    assert(runtime.read(4, 1, value) && value == 0xAB);
    assert(runtime.write(8, 2, 0xBEEF));
    assert(runtime.read(8, 2, value) && value == 0xBEEF);
    assert(runtime.write(12, 4, 0x12345678));
    assert(runtime.read(12, 4, value) && value == 0x12345678);

    assert(!runtime.read(255, 2, value));
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

    return 0;
}
