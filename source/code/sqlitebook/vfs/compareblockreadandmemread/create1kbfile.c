#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    int ret = 0;
    int blockfilefd = open("blockfile", O_CREAT|O_WRONLY);
    for (int i = 0; i < 1024; i++) {
	    ret = write(blockfilefd, &i, 1);
    }
    close(blockfilefd);
}
