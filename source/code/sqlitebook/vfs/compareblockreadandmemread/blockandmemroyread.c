#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    int ret = 0;
    void *blockcache = malloc(1024);
    void *memcache = malloc(1024);
    int blockfilefd = open("blockfile", O_RDONLY);
    for (int i = 0; i < 1024; i++) {
	lseek(blockfilefd, i, SEEK_SET);
        read(blockfilefd, blockcache + i, 1);
    }
    close(blockfilefd);
}
