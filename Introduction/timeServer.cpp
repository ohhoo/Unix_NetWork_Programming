#include <time.h> 
#include "unp/unp.h" 
#include "../error_unp.h"


int main(int argc, char **argv)
{
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	char buff[MAXLINE];
	time_t ticks;

	//创建套接字
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	//指定IP地址为INADDR_ANY,这样如果服务器主机有多个接口，服务器进程就可以在任意网络接口上接受连接
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//servaddr.sin_port        = htons(13);//端口要与timeClient中要连接的端口一致
	servaddr.sin_port        = htons(9999);

	//将端口绑定至创建的socket
	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	//将创建的socket转换为监听socket，LISTENQ表示队列中允许容纳等待连接最大的连接数
	listen(listenfd, LISTENQ);

	for(;;) {//持续等待建立连接、写入数据，完成后断开连接等待下一次连接建立
		//建立连接，接受连接的过程由内核完成
		connfd = accept(listenfd, (SA *) NULL, NULL);

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        //一次性写入数据
		//write(connfd, buff, strlen(buff));
		//循环写入
		int buffLen = strlen(buff);
		int len = 0;
		while(len++<buffLen){
			write(connfd,buff+len,1);
		}
		//关闭连接
		close(connfd);
	}
}