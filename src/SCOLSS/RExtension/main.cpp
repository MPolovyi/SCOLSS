#include <iostream>
#include <dlfcn.h>
#include <string.h>

#include "PtInput.h"

#include "ExtensionDefs.h"

int main() {
    void* handle = dlopen("../build/lib/libRExtensionLibrary.so", RTLD_LAZY);

    int ptCount = 5000;
    double neigh_c[5000];
    double coords[5000];
    double systemSize = 6000;

    int ptStrLength = 320000;
    int timePointsCount = 3;

    Function_GetDynamicChains(neigh_c, coords, &encoded, &ptStrLength, &timePointsCount, &ptCount, &systemSize);

    return 0;
}
