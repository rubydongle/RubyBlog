#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <termios.h>  // TIOCSCTTY

int main() {
        int len;
        char recv_buf[1000] = {0};

        int mfd = open("/dev/ptmx",O_RDWR);
        char* ptsname = malloc(100);
        ptsname_r(mfd, ptsname, 100);
        printf("ptsname is %s\n", ptsname);
        grantpt(mfd);
        unlockpt(mfd);

        int pid = fork();

        if (pid == 0) {
            printf("pid:%d child start\n", getpid());
                // sub process
            setsid();
            int sfd = open(ptsname, O_RDWR);
            ioctl(sfd,  TIOCSCTTY, 0);

            dup2(sfd, 0);
            dup2(sfd, 1);
            dup2(sfd, 2);

            printf("pid:%d stdio redirect to sfd\n",getpid());

		struct termios termiop;
		tcgetattr(sfd, &termiop);
		printf("sfd termio disclipline %c\n", termiop.c_line);

            char *argv[]={"ls","-l",(char *)0};
            execv("/usr/bin/ls", argv);
            exit(-1);
        } else {
                len = read(mfd, recv_buf, sizeof(recv_buf));
                printf("pid:%d ------------------\n", getpid());
                printf("pid:%d read mfd len:%d recv_buf:\n[%s]\n",getpid(), len, recv_buf);

        //struct termio {
        //unsigned short c_iflag;         /* input mode flags */
        //unsigned short c_oflag;         /* output mode flags */
        //unsigned short c_cflag;         /* control mode flags */
        //unsigned short c_lflag;         /* local mode flags */
        //unsigned char c_line;           /* line discipline */
        //unsigned char c_cc[NCC];        /* control characters */
        //};


		struct termios termiop;
		tcgetattr(mfd, &termiop);
		printf("mfd termio c_iflag:0x%x disclipline:%c\n", termiop.c_iflag,termiop.c_line);

        }
}
