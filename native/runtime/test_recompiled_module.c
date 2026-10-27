#include "recomp.h"

static void test_guest_function(RCtx *ctx) {
    ctx->r[0] += 7;
}

static const RFuncEntry test_table[] = {
    {0x3000u, 0u, test_guest_function},
};

int rc_code_init(const RHost *host, RCodeInfo *info) {
    if (host == 0 || info == 0) {
        return 0;
    }
    if (host->version != RC_HOST_VERSION || host->ctx_size != sizeof(RCtx)) {
        return 0;
    }

    memset(info, 0, sizeof(*info));
    info->table = test_table;
    info->count = (u32)(sizeof(test_table) / sizeof(test_table[0]));
    info->text_base = 0x3000u;
    info->text_size = 4u;
    info->text_crc32 = 0x4D483455u;
    info->title_id = 0x0004000000123400ull;
    return 1;
}
