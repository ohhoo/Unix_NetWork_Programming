#include "unp.h"

int main(int argc, char* argv[]){
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc != 2)
        err_quit("usage: client <IPAddress>");
    
    //创建socket
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    //给服务端协议地址结构赋值
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    //建立连接
    Connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
    char welcome[40];
    read(sockfd,welcome,sizeof(welcome)-1);

    printf("%s\n",welcome);

    str_cli(stdin, sockfd);

    exit(0);

    return 0;
}