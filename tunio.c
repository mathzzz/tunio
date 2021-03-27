#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

struct frame {
        unsigned int len;
        char data[2000];
};

// 为了便于重定向操作，我在本程序中只使用了3个文件描述符：TAP网卡字符设备，标准输入，标准输出
int main(int argc, char **argv)
{
        int fd = -1;
        struct ifreq ifr;
        size_t len;
        char buf[2004];
        struct frame frm;
        int i;
        char *tap = "tap0";
        if(argv[1]) tap = argv[1];

        fd_set rd_set;

        if( (fd = open("/dev/net/tun", O_RDWR)) < 0) {
                exit(-1);
        }
        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags |= IFF_NO_PI;
        ifr.ifr_flags |= IFF_TAP;
        snprintf(ifr.ifr_name, IFNAMSIZ, "%s", tap);
        ioctl(fd, TUNSETIFF, (void *)&ifr);
        while(1) {
                int nfds;
                int j;
                FD_ZERO(&rd_set);
                FD_SET(0, &rd_set);
                FD_SET(fd, &rd_set);

                nfds = select(1024, &rd_set, NULL, NULL, NULL);
                for (j = 0;j < nfds; j++) {
                        // 如果标准输入有动静的话
                        if(FD_ISSET(0, &rd_set)) {
                                unsigned int dlen;
                                // 从标准输入中读取数据长度
                                len = read(0, &dlen, sizeof(unsigned int));
                                // 按照指示的长度读取原始IP报文或者数据帧
                                len = read(0, frm.data, dlen);
                                // 将IP报文或者数据帧写入TAP设备
                                len = write(fd, frm.data, len);
                        }
                        // 如果虚拟网卡字符设备有动静
                        if(FD_ISSET(fd, &rd_set)) {
                                // 从网卡读取IP报文或者以太帧
                                len = read(fd, buf, sizeof(buf));
                                // 为了对端可以区分数据边界，加入了长度头
                                frm.len = len;
                                memcpy(frm.data, buf, len);
                                // 将加入长度头的原始数据输出到标准输出
                                write(1, &frm, len+sizeof(int));
                        }
                }
        }
        return 0;
}
