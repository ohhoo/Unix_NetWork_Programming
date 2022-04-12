#include "unp.h"

void sig_chld(int signo);

int main(int argc, char* argv[]){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0); //创建监听socket

    bzero(&servaddr, sizeof(servaddr));
    //为监听socket指定相应的协议、IP、端口信息
    servaddr.sin_family = AF_INET;  
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    //将协议地址绑定至监听socket
    Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));
    //打开监听socket的接受连接功能，并设定连接队列长度
    Listen(listenfd, LISTENQ);

    //增加信号处理函数
    Signal(SIGCHLD, sig_chld);//当出现SIGCHLD信号时，会调用处理函数sig_chld
    //建立信号处理函数必须在fork之前，并且只做一次

    for(;;){
        clilen = sizeof(cliaddr);
        //接收已完成的连接，生成新的socket
        // connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
        //accept新创建的socket使用与监听socket相同的端口号
        //但是连接是由服务器及客户端的socket共同确定，端口号只起到标识应用的作用。

        if((connfd = accept(listenfd, (SA*)&servaddr, &clilen)) < 0){
            if(errno == EINTR)
                continue;//当前进程阻塞于一个慢系统调用()
                //在捕获某个信号且相应信号处理函数返回时，阻塞的慢系统调用可能会返回一个EINTR错误
                //在此处我们忽略该错误，并重新启动accept
            else
                err_sys("accept error!");
        }

        if( (childpid = Fork()) == 0 ){//进入子进程
            Close(listenfd);//关闭监听socket(将listenfd的引用计数减一)
            str_echo(connfd);//回显、处理
            exit(0);//退出子进程，同时会关闭所有打开的描述符(connfd的引用计数会减一)
        }
        Close(connfd);//关闭连接socket
    }
}

void sig_chld(int signo){
    pid_t pid;
    int stat;

    //该处理函数中使用了while循环调用waitpid
    //指定了WNOHANG选项让waitpid函数在尚有子进程在运行时不要阻塞，而相对的wait就无该选项
    //因此wait不能全部处理僵死进程
    while((pid = waitpid(-1,&stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}