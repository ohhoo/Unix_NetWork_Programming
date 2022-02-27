#include "unp/unp.h"
//引入错误信息输出的相关函数的头文件，该头文件必须在unp.h后引入
#include "../error_unp.h"


int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 2) {
        err_quit("usage: a.out <IPaddress>");
    }
    //创建了一个流式套接字，TCP提供底层通信，返回一个整数值标识该创建的套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }
    printf("socket id = %d\n",sockfd);

    //执行清除操作，将servaddr结构初始化为0
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //servaddr.sin_port   = htons(13);//得到的端口号为3328
    servaddr.sin_port   = htons(9999);

    //将接收到的点分十进制IP地址转换为二进制整数
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        err_quit("inet_pton error for %s", argv[1]);
    }

    //建立从sockfd标识的本地socket到servaddr表示的远程套接字的连接，第三个参数为套接字地址结构的长度
    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    printf("port = %d\n",servaddr.sin_port);

    int count = 0;
    //从套接字内读取数据，当read返回值为0或负数时读取结束
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        ++count;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
    printf("read > 0:%d\n",count);
    if (n < 0) {
        err_sys("read error");
    }

    exit(0);
}