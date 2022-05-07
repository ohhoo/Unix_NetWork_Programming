#include "unp.h"

void dg_echo(int fd, struct sockaddr* sa, socklen_t clilen);

int main(int argc, char* argv[]){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(sockfd, (SA*)&servaddr, sizeof(servaddr));//将socket绑定至端口

    dg_echo(sockfd, (SA*)&cliaddr, sizeof(cliaddr));

    return 0;
}


void dg_echo(int fd, struct sockaddr* sa, socklen_t clilen){
    int n;
    socklen_t len;
    char msg[MAXLINE];

    for(;;){
        len = clilen;
        n = Recvfrom(fd, msg, MAXLINE, 0, sa, &len);//接收数据
        Sendto(fd, msg, n, 0, sa, len);//发送数据
    }
}