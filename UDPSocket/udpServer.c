#include "unp.h"

void dg_echo(int fd, struct sockaddr* sa, socklen_t clilen);
//测试流量控制
void dg_echo_large_data(int fd, struct sockaddr* sa, socklen_t clilen);
static void recvfrom_int(int);
static int count = 0;

int main(int argc, char* argv[]){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(sockfd, (SA*)&servaddr, sizeof(servaddr));//将socket绑定至端口

    //这是一个迭代的服务器程序，通常来讲udp服务器都是迭代的，而TCP服务器都是并发的
    //dg_echo(sockfd, (SA*)&cliaddr, sizeof(cliaddr));
    dg_echo_large_data(sockfd, (SA*)&cliaddr, sizeof(cliaddr));

    printf("count: %d", count);
    return 0;
}


void dg_echo(int fd, struct sockaddr* sa, socklen_t clilen){
    int n;
    socklen_t len;
    char msg[MAXLINE];

    for(;;){
        len = clilen;
        n = Recvfrom(fd, msg, MAXLINE, 0, sa, &len);//接收数据，并获取数据来源的socket地址信息
        Sendto(fd, msg, n, 0, sa, len);//发送数据到数据来源socket地址
    }
}

void dg_echo_large_data(int fd, struct sockaddr* sa, socklen_t clilen){
    socklen_t len;
    char msg[MAXLINE];

    //信号触发
    Signal(SIGINT, recvfrom_int);

    //可以设定socket的接收缓冲区大小
    int n = 220 * 1024;
    Setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));

    for(;;){
        //收到大量的数据报后缓冲区的大小可能显得不足,可以重新设置缓冲区大小
        len = clilen;
        Recvfrom(fd, msg, MAXLINE, 0, sa, &len);
        count++;
    }
}

static void recvfrom_int(int signo){
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}
