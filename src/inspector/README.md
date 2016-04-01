## ForestDB Block Inspector
Block inspector analyzes a type of a block. 
Inputs are outputs of `blktrace`

## HOW TO BUILD

Only works on Linux platforms. There is an dependency on forestdb source.
You may need to modify the codes of inspector related to your forestdb version.

- Use preferable c++ compiler to compile sourcode. 
- For standalone fdb-inspector, compile `standalone.cc` file
with header file directory options. (e.g. g++ -I/home/me/forestdb/include)
    - Header file directories are /your/fdb/home/include, 
    /your/fdb/home/src, /your/fdb/home/utils

## HOW TO RUN
Let's assume that binary file is `fdb-analyzer`

Run the binary with blktrace before running your forestdb benchmark on background. 
`blktrace` and `fdb-analyzer` should be run as a root previlege. 
```
    $> sudo btrace /dev/sdc 2>&1 | 
             sudo ./fdb-analyzer -d /dev/sdc -m 0 -b 4096 &> trace &
    $> ./runBenchamrk.sh
```

After the benchmark finished, kill `blktrace` or `fdb-analyzer`.
```
    $> sudo killall -15 btrace blktrace
    $> sudo killall fdb-analyzer
```

### Options
- `d`: Specify your device name (You must always use this option)
- `b`: Specify your forestdb's block size in bytes (default: 4096)
- `m`: Specify what metadata your forestdb uses (default: 0)
    - `0`: BLK_MARKER is used
    - `1`: document metadata structure is used (since MAGIC_02 version of forestdb)

## RESULTS
`fdb-analyzer` prints below informations. 
- Output of `blktrace` itself
- For write requests, what forestdb block types are written
    - `DBHEADER`, `BNODE`, `DOC`, `SUPERBLOCK (SB)`
    - `Unknown` for the others

At the end of the benchmark, `fdb-analyzer` prints 
- Total block count for each forestdb block types
- Ratio of each forestdb block types
