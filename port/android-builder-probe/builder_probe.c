#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname info;
    if (uname(&info) != 0) {
        perror("uname");
        return 2;
    }

    printf("MH4U_ANDROID_BUILDER_PROBE=OK\n");
    printf("machine=%s\n", info.machine);
    printf("pid=%ld\n", (long)getpid());
    printf("message=Android launched an ARM64 PIE executable from nativeLibraryDir\n");
    return 0;
}
