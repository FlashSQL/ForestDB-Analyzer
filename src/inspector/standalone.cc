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

#define __GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <signal.h>

using namespace std;

#define SUCCESS         0x00
#define UNKNOWN_ERROR   0x01
#define UNKNOWN_ARG     0x02 
#define DEV_NOT_FOUND   0x04

void print_usage(char* name) {
    fprintf(stderr, "Usage: %s options\n", name);
    fprintf(stderr, "-b blocksize: block size of target forestdb (default: 4096)\n");
    fprintf(stderr, "-m meta: 0 - use BLK_MARKER, 1 - use DOCBLK_META\n");
    fprintf(stderr, "                (default: 0)\n");
    fprintf(stderr, "-d devname: Device name which contains target forestdb\n");
    fprintf(stderr, "            (/dev/sdxx must be used)\n");
}

/**
  * Use printf("%" PRIu64 "\n", blocksize); to print uint64_t variables.
  */
struct config {
    int marker;
    uint64_t blocksize;
    char* devname;
};

struct config global_config;

void init_config(void) {
    global_config.marker = 0;
    global_config.blocksize = 0;
    global_config.devname = (char*)malloc(sizeof(char) * 512);
}

void destroy_config(void) {
    free(global_config.devname);
}

int verify_config(void) {
    if (0 >= strlen(global_config.devname)) {
        return DEV_NOT_FOUND;
    }

    if (0 >= global_config.blocksize) {
        global_config.blocksize = 4096;
    }

    if (0 != global_config.marker && 1 != global_config.marker) {
        global_config.marker = 0;
    }

    /* print options */
    fprintf(stderr, "====== Configurations ======\n");
    fprintf(stderr, "Blocksize: %" PRIu64 " bytes\n", global_config.blocksize);
    fprintf(stderr, "Device name: %s\n", global_config.devname);
    fprintf(stderr, "Marker: %d\n", global_config.marker);
    fprintf(stderr, "============================\n");
    return SUCCESS;
}

/** 
  * extern int optind, opterr, optopt;
  * extern char *optarg;
  */
int parse_opt(int argc, char* argv[]) {
    const char *opt = "b:d:m:";
    char option;
    
    optind = 1;
    while ( -1 != (option = getopt(argc, argv, opt)) ) {
        switch (option) {
            case 'b' :
                global_config.blocksize = atoll(optarg);
                break;
            case 'd' : 
                strcpy(global_config.devname, optarg);
                break;
            case 'm' :
                global_config.marker = atoi(optarg);
                break;
            default:
                return UNKNOWN_ARG;
        }; // switch (option)
    }; // while ( getopt() ) 
    return SUCCESS;
}

/**
  * Block marker of forestdb
  * (commit version is 671823e363615f37439f3ddaaeb730018a3db0a4) 
  */
#define BLK_MARKER_BNODE (0xff)
#define BLK_MARKER_DBHEADER (0xee)
#define BLK_MARKER_DOC (0xdd)
#define BLK_MARKER_SB (0xcc) // superblock
#define BLK_MARKER_SIZE (1)

enum fdb_blk_types {
    DOC = 0,    /* document */
    BNODE,      /* index node */ 
    DBHEADER,   /* db header */
    SB,         /* superblock */
    BLK_TYPE_END
};

char fdb_blk_names[BLK_TYPE_END + 2][16] = {
    "Document",
    "Index",
    "Header",
    "Superblock",
    "Not FDB",
    "Total Writes"
};

uint64_t type_cnt[BLK_TYPE_END + 2]; //this includes total count 


void print_count(void) {
    uint64_t total = type_cnt[BLK_TYPE_END+1];
    uint64_t fdb_total = 
        total - type_cnt[BLK_TYPE_END];
    
    fprintf(stderr, "\n======== Statistics =========\n");
    for (int i = 0 ;i < BLK_TYPE_END + 2;i++) {
        fprintf(stderr, "TYPE %s: %" PRIu64 
                " pages, %.4Lf%% of FDB, %.4Lf%% of Total\n", 
                fdb_blk_names[i], type_cnt[i], 
                (long double)type_cnt[i] / fdb_total * 100,
                (long double)type_cnt[i] / total * 100);
    }
    fprintf(stderr, "=============================\n");
}

int do_inspect(string& appname) {
    int dev;
    int blocksize = global_config.blocksize;
    int metasize = 
        (global_config.marker == 0?
         BLK_MARKER_SIZE: 1);
    int idx = 0;
    uint64_t offset, bytes;
    char *block; 
    char tline[128];
    unsigned char blk_type;
    string line, output;
    string *lists;
    char scanbuf[1024];


    if ( 0 >= (dev = open(global_config.devname, O_RDONLY | O_LARGEFILE))) {
        fprintf(stderr, "Errno: %d, %s\n", errno, strerror(errno));
        return DEV_NOT_FOUND;
    } // open device 

    memset(type_cnt, 0, sizeof(type_cnt));
    lists = new string[16];
    block = (char*)malloc(sizeof(char) * blocksize);

    while ( fgets(tline, sizeof(tline)-1, stdin) ) {
        line = string(tline);

        istringstream is(line); /* prepare tokenizing */
        idx = 0;

        /* get tokens */
        while ( idx < 16 && is >> lists[idx++] );

        if ( idx < 14 && idx > 10 && lists[10].substr(1, appname.size()) == appname ) {
            /* Skip for the I/O issued by this inspector */
            continue;
        }
        if ( idx > 10 && 
                (lists[5][0] == 'D' && (lists[6][0] == 'W' || lists[6][0] == 'R')) ||
                (lists[5][0] == 'I' && lists[6][0] == 'F')) {
            /* get offset and num sectors */
            offset = atoll(lists[7].c_str()) * 512;
            bytes = atoll(lists[9].c_str()) * 512;

            lists[9] = string("8");
            /* inspect a block */
            while ( blocksize <= bytes && 0 < bytes ) {
                pread(dev, block, blocksize, offset);

                if (global_config.marker) {
                } else {
                    blk_type = *(block + blocksize - metasize);
                } // if marker 

                switch (blk_type) {
                    case BLK_MARKER_BNODE:
                        output = string("BNODE");
                        type_cnt[BNODE]++;
                        break;
                    case BLK_MARKER_DBHEADER:
                        output = string("DBHEADER");
                        type_cnt[DBHEADER]++;
                        break;
                    case BLK_MARKER_DOC:
                        output = string("DOC");
                        type_cnt[DOC]++;
                        break;
                    case BLK_MARKER_SB:
                        output = string("SB");
                        type_cnt[SB]++;
                        break;
                    default:
                        output = string("NOT_FDB");
                        type_cnt[BLK_TYPE_END]++;
                        break;
                }; //switch blk_type
                type_cnt[BLK_TYPE_END+1]++;

                //new offset for separation 
                sprintf(scanbuf, "%"PRIu64, offset);
                lists[7] = string(scanbuf);

                // Next offset
                offset += blocksize;
                bytes -= blocksize;
                for (int k = 0 ;k < idx;k++) {
                    if (lists[k] == string("00") ) {
                        continue;
                    }
                    cout << lists[k] << " ";
                } // print 
                cout << output << endl; ;

                /* print only once for FWFS */
                if (lists[6][0] == 'F') {
                    break;
                } // FWFS
            } // while (blocksize <= bytes && 0 < bytes)
        } else { 
            cout << line;
        } // if D W 
    } //while ( fgets() )

    print_count();

    delete[] lists;
    free(block);
    return SUCCESS;
}
void handle_signal(int num) {
    printf("== signal %d caught ==\n", num);
    //print_count();
}
void set_signal(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
}

int main(int argc, char *argv[]) {
    int rv;
    string appname; 

    init_config();

    if( SUCCESS != (rv = parse_opt(argc, argv)) ) {
        print_usage(argv[0]);
        goto exit;
    } //parse options 

    if( SUCCESS != (rv = verify_config()) ) {
        print_usage(argv[0]);
        goto exit;
    } // verify configs

    set_signal();

    appname = string(basename(argv[0])); 

    rv = do_inspect(appname);

exit:
    destroy_config();
    return rv;
}
