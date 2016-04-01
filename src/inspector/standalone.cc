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


#define SUCCESS         0x00
#define UNKNOWN_ERROR   0x01
#define UNKNOWN_ARG     0x02 
#define DEV_NOT_FOUND   0x04

void print_usage(char* name) {
    printf("Usage: %s -b blocksize -d devname\n", name);
    printf("blocksize: block size of target forestdb\n");
    printf("devname: Device name which contains target forestdb\n");
    printf("         (/dev/sdxx)\n");
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
    global_config.devname = (char*)malloc(sizeof(char) * 512);
}

void destroy_config(void) {
    free(global_config.devname);
}

int verify_config(void) {
    if(0 >= global_config.blocksize) {
        global_config.blocksize = 4096;
    }
    if(0 >= strlen(global_config.devname)) {
        return DEV_NOT_FOUND;
    }

    printf("====== Configurations ======\n");
    printf("Blocksize: %" PRIu64 " bytes\n", global_config.blocksize);
    printf("Device name: %s\n", global_config.devname);
    printf("============================\n");
    return SUCCESS;
}

/**
  * Block marker of forestdb
  * (commit version is 671823e363615f37439f3ddaaeb730018a3db0a4) 
  *
  * #define BLK_MARKER_BNODE (0xff)
  * #define BLK_MARKER_DBHEADER (0xee)
  * #define BLK_MARKER_DOC (0xdd)
  * #define BLK_MARKER_SB (0xcc) // superblock
  * #define BLK_MARKER_SIZE (1)
  * #define DOCBLK_META_SIZE (16)
  * struct docblk_meta {
  *     bid_t next_bid;
  *     uint16_t sb_bmp_revnum_hash;
  *     uint8_t reserved[5];
  *     uint8_t marker;
  * };
  */

int do_inspect(void) {
    return SUCCESS;
}

/** 
  * extern int optind, opterr, optopt;
  * extern char *optarg;
  */
int parse_opt(int argc, char* argv[]) {
    const char *opt = "b:d:";
    char option;
    
    optind = 1;
    while ( -1 != (option = getopt(argc, argv, opt)) ) {
        switch (option) {
            case 'b' :
                global_config.blocksize = atoi(optarg);
                break;
            case 'd' : 
                strcpy(global_config.devname, optarg);
                break;
            default:
                return UNKNOWN_ARG;
        };
    };
    return SUCCESS;
}
int main(int argc, char *argv[]) {
    int rv;

    init_config();

    if( SUCCESS != (rv = parse_opt(argc, argv)) ) {
        print_usage(argv[0]);
        goto exit;
    }

    if( SUCCESS != (rv = verify_config()) ) {
        print_usage(argv[0]);
        goto exit;
    }

    rv = do_inspect();

exit:
    destroy_config();
    return rv;
}
