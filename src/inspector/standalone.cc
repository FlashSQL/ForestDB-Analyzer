/** 
  * Standalone ForestDB block inspector 
  * - Latest forestdb source codes are needed. 
  * 
  * written by Gihwan oh 
  */

/**
  * "src/common.h" header file of forestdb contains types of block marker
  * and docblk_meta structure.
  */
#include "src/common.h" 
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

using namespace std;

void print_usage(char* name) {
    printf("Usage: %s blocksize\n", name);
    printf("blocksize: block size of target forestdb\n");
}

/**
  * Use printf("%" PRIu64 "\n", blocksize); to print uint64_t variables.
  */
struct config {
    uint64_t blocksize;
    char* devname;
};

struct config global_config;

void init_config(void) {
    global_config.blocksize = 0;
    global_config.devname = NULL;
}

int do_inspect(void) {
}

int main(int argc, char *argv[]) {
    int rv;

    init_config();
    rv = do_inspect();

    return rv;
}
