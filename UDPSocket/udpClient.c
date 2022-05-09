#include "unp.h"

void dg_cli(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen);

//连接标准echo服务
void dg_cli_stander(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen);

//使用connect建立连接
void dg_cli_connect(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen);

//发送大量数据，测试流量控制
#define NDG 1999
#define DGLEN 1400
void dg_cli_large_data(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen);


int main(int argc, char* argv[]){
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc != 2)
        err_quit("Usage: udp_c.o <IPAddress>");
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    //测试标准回射服务
    /*
    使用标准回射服务需要查看该服务是否打开，在/etc/xinetd.d/echo-udp中设置
    重启xineted服务即可打开标准回射
    当服务未打开时，客户端将会阻塞于Recvfrom调用
    */
    //servaddr.sin_port = htons(7);


    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    printf("sockfd : %d", sockfd);
    dg_cli(stdin, sockfd, (SA*)&servaddr, sizeof(servaddr));
    //dg_cli_stander(stdin, sockfd, (SA*)&servaddr, sizeof(servaddr));
    //dg_cli_connect(stdin, sockfd, (SA*)&servaddr, sizeof(servaddr));
    //dg_cli_large_data(stdin, sockfd, (SA*)&servaddr, sizeof(servaddr));
    exit(0);
}


void dg_cli(FILE* p, int sockfd, const struct sockaddr* to, socklen_t servlen){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    while(Fgets(sendline, MAXLINE, p) != NULL){//标准输入将数据输入待发送区
        Sendto(sockfd, sendline, strlen(sendline), 0, to, servlen);//向服务器发送数据

        n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);//接收，不关注数据从何而来因此后两个参数赋值为NULL

        recvline[n] = 0;

        Fputs(recvline, stdout);//输出接收到的数据
    }
}

void dg_cli_stander(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    socklen_t len;
    struct sockaddr* preply_addr;

    preply_addr = malloc(servlen);
    
    printf("enter dg_cli_stander");
    
    while(Fgets(sendline, MAXLINE, p) != NULL){
        Sendto(sockfd, sendline, MAXLINE, 0, from, servlen);

        len = servlen;

        n = Recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);

        if(len != servlen || memcpy(from, preply_addr, len) != 0){
            printf("replay from %s (ignored)\n", Sock_ntop(preply_addr, len));
            continue;
        }

        recvline[n] = 0;
        Fputs(recvline, stdout);
    }
}


void dg_cli_connect(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    //建立连接
    Connect(sockfd, (SA*)from, servlen);

    while(Fgets(sendline, MAXLINE, p) != NULL){
        Write(sockfd, sendline, strlen(sendline));

        n = Read(sockfd, recvline, MAXLINE);

        recvline[n] = 0;

        Fputs(recvline, stdout);
    }
}

void dg_cli_large_data(FILE* p, int sockfd, const struct sockaddr* from, socklen_t servlen){
    int i;
    char sendline[DGLEN];

    //如果使用本地环回地址127 很大可能看不到丢包现象，数据并不会过网卡发送
    for(i = 0;i < NDG;i++)
        Sendto(sockfd, sendline, DGLEN, 0, from, servlen);
}
