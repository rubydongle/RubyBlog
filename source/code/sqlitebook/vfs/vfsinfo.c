
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>

int main(int argc, char **argv)
{

    printf("cmd:%s arg1:%s\n", argv[0], argv[1]);

    if (argv[1] == NULL) {
        printf("must pass a file\n");
        exit(1);
    }

    struct statvfs fsInfo;

    statvfs(argv[1], &fsInfo);

    //       struct statvfs {
    //          unsigned long  f_bsize;    /* Filesystem block size */
    //           unsigned long  f_frsize;   /* Fragment size */
    //           fsblkcnt_t     f_blocks;   /* Size of fs in f_frsize units */
    //           fsblkcnt_t     f_bfree;    /* Number of free blocks */
    //           fsblkcnt_t     f_bavail;   /* Number of free blocks for
    //                                         unprivileged users */
    //           fsfilcnt_t     f_files;    /* Number of inodes */
    //           fsfilcnt_t     f_ffree;    /* Number of free inodes */
    //           fsfilcnt_t     f_favail;   /* Number of free inodes for
    //                                         unprivileged users */
    //           unsigned long  f_fsid;     /* Filesystem ID */
    //           unsigned long  f_flag;     /* Mount flags */
    //          unsigned long  f_namemax;  /* Maximum filename length */
    //      };

    printf("FileSystem Block/Sector Size:%lu\n", fsInfo.f_bsize);
    printf("Fragment size:%lu\n", fsInfo.f_frsize);
    printf("Size of fs in f_frsize units:%lu\n", fsInfo.f_blocks);
    printf("Number of free blocks:%lu\n", fsInfo.f_bfree);

}
