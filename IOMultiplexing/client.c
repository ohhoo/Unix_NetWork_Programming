#include "unp.h"
#include <stdio.h>

//重新定义新版本的str_cli
void str_cli1(FILE* fp, int sockfd);

//功能健壮的str_cli函数
void str_cli2(FILE* fp, int sockfd);


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

    str_cli2(stdin, sockfd);

    exit(0);

    return 0;
}

void str_cli1(FILE* fd, int sockfd){
    //当将标准的输入输出重定向到文件时，会出现输出文件总小于输入文件的问题(会有文件内容遗漏在缓冲区)
    int maxfdp;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&rset);//初始化描述符集合

    for(;;){
        FD_SET(fileno(fd), &rset);//设置描述符对应的位为1
        FD_SET(sockfd, &rset);
        maxfdp = max(fileno(fd), sockfd) + 1;
        
        //程序会阻塞在select处，直到有描述符就绪
        Select(maxfdp, &rset, NULL, NULL, NULL);//最后一个NULL标识了Select函数将会一直等待直到描述符I/O就绪

        if(FD_ISSET(sockfd, &rset)) { //socket描述符就绪
            if(Readline(sockfd, recvline, MAXLINE) == 0)//当对端的进程终止，返回0值
                err_quit("str_cli: server terminated prematurely");
            
            Fputs(recvline, stdout);//输出
        }

        if(FD_ISSET(fileno(fd), &rset)){ //文件描述符就绪
            if(Fgets(sendline, MAXLINE, fd) == NULL)
                return;
            Writen(sockfd, sendline, strlen(sendline));//输入
        }
    }
}


void str_cli2(FILE* fd, int sockfd){
    //这一版的函数可正确处理批量输入
    int maxfd1, stdineof;
    fd_set rset;
    char buf[MAXLINE];
    int n;

    stdineof = 0;
    FD_ZERO(&rset);

    for(;;){
        if(stdineof == 0)
            FD_SET(fileno(fd), &rset);
        
        FD_SET(sockfd, &rset);
        maxfd1 = max(fileno(fd), sockfd) + 1;
        Select(maxfd1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)){//socket读就绪
            if((n = Read(sockfd, buf, MAXLINE)) == 0){//改用Read对缓冲区进行读取，而不是用文本行
                if(stdineof == 1)
                    return;//当socket中读到EOF时，说明结束
                else
                    err_quit("str_cli: server terminated prematurely");
            }

            Write(fileno(stdout), buf, n);
        }

        if(FD_ISSET(fileno(fd), &rset)){
            if((n = Read(fileno(fd), buf, MAXLINE)) == 0){
                stdineof = 1;//在标准输入上碰到EOF时，关闭连接的写部分
                Shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fd), &rset);//从文件描述符集合rset中删除fd对应的文件描述符
                continue;
            }
            Writen(sockfd, buf, n);
        }
    }
}
