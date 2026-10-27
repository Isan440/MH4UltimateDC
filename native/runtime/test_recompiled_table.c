#include "recomp.h"

/* Mirrors the symbols emitted by tools/recompile.py's generated table.c. */
static void test_guest_function(RCtx *ctx) {
    ctx->r[0] += 7;
    /* Exercise rc_dll.c's fallback from generated code to the host runtime. */
    rc_call(ctx, 0x1000u);
}

const RFuncEntry rc_func_table[] = {
    {0x3000u, 0u, test_guest_function},
};
const u32 rc_func_count = 1u;
const u32 rc_text_base = 0x3000u;
const u32 rc_text_size = 4u;
const u32 rc_text_crc32 = 0x4D483455u;
const u64 rc_title_id = 0x0004000000123400ull;
const u32 rc_hook_addrs[] = {0u};
const u32 rc_hook_count = 0u;
RHookFn rc_hooks[1];
u32 rc_tunables[] = {0u};
const u32 rc_tunable_addrs[] = {0u};
const char *const rc_tunable_names[] = {0};
const u32 rc_tunable_count = 0u;
