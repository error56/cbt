#include <stdio.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname u;

    if (uname(&u) == -1) {
        perror("uname");
        return 1;
    }

    printf("sysname:  %s\n", u.sysname);
    printf("nodename: %s\n", u.nodename);
    printf("release:  %s\n", u.release);
    printf("version:  %s\n", u.version);
    printf("machine:  %s\n", u.machine);

    return 0;
}
