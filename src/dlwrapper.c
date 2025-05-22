//gcc -o dlwrapper.so -shared -pthread -rdynamic -Wl,-wrap,dlopen -m64 -ldl -fPIC  src/dlwrapper.c
// LD_DEBUG=versions LD_PRELOAD=./dlwrapper.so node example.js
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

void *dlopen(const char *filename, int flags) {
//https://gist.github.com/aprell/b3c6e7877cf1a228a0166d0065a792ad
    void *(*real_dlopen) (const char *filename, int flags);
    *(void **)&real_dlopen = dlsym(RTLD_NEXT, "dlopen");
    printf("# dlopen: %s\n", filename);
    if (flags & RTLD_LAZY) {
        printf("RTLD_LAZY ");
    }
    if (flags & RTLD_NOW) {
        printf("RTLD_NOW ");
    }
    if (flags & RTLD_GLOBAL) {
         printf("RTLD_GLOBAL ");
    }
    if (flags & RTLD_LOCAL) {
         printf("RTLD_LOCAL ");
    }
    if (flags & RTLD_NODELETE) {
         printf("RTLD_NODELETE ");
    }
    if (flags & RTLD_NOLOAD) {
         printf("RTLD_NOLOAD ");
    }
    if (flags & RTLD_DEEPBIND) {
         printf("RTLD_DEEPBIND ");
    }
    printf("\n");
    void *result;
    result = real_dlopen(filename, flags);
    if (result == 0) {
        printf("RESULT NULL\n");
        printf("%s\n", dlerror());
    } else {
        printf("RESULT NOTNULL\n");
    }

    return result;
}

