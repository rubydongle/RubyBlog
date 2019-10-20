
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

    printf("FileSystem Block/Sector Size:%lu\n", fsInfo.f_bsize);

}
