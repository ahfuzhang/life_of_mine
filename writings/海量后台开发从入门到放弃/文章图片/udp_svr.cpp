#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void test_udp_svr(const char* ip, unsigned short port){
    int udp_svr = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1==udp_svr){
        printf("socket error:%d %s\n", errno, strerror(errno));
        return;
    }
    //
    int buffer_size = 1024*64;
    setsockopt(udp_svr, SOL_SOCKET, SO_RCVBUF, (char*)&buffer_size, sizeof(int));
    //
    struct timeval st;
    st.tv_sec = 1;
    st.tv_usec = 0;
    setsockopt(udp_svr, SOL_SOCKET, SO_RCVTIMEO, (char*)&st, sizeof(struct timeval));
    //
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    int ret = bind(udp_svr, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (-1==ret){
        printf("bind error:%d %s\n", errno, strerror(errno));
        close(udp_svr);
        return;
    }
    //
    char buf[1024*64];
    struct sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(struct sockaddr);
    for (;;){
        int recv_len = recvfrom(udp_svr, buf, sizeof(buf), 0, (struct sockaddr*)&remote_addr, &addr_len);
        if (recv_len<=0){
            continue;
        }
        //»Ø°ü
        int send_len = sendto(udp_svr, buf, recv_len, 0, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr));
        if (send_len<recv_len){
            continue;
        }
        //todo: stat code is here
    }
}

int main(int argc, char* argv[]){
    if (argc<3){
        printf("usage:%s <ip> <port>\n", argv[0]);
        return 0;
    }
    const char* ip = argv[1];
    unsigned short port = strtoul(argv[2], NULL, 10);
    if (port<1000){
        printf("port max bigger 1000\n");
        return 0;
    }
    test_udp_svr(ip, port);
    return 1;
}

/*
g++ -o udp_svr.o -c udp_svr.cpp -g -Wall -Werror 
g++ -o udp_svr udp_svr.o 
*/
