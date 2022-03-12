# 本章节介绍在Unix环境下进行网络编程时会用到的API
## socket地址结构
IPv4版本
```c
struct in_addr{
    in_addr_t s_addr;   //实际是一个32位无符号int型数据，存储32位的网络地址
}

struct sockaddr_in{
    uint8_t sin_len;        //无符号的8bit char 表示该结构的长度
    sa_family_t sin_family; //无符号的short 8bit 表示socket版本
    int_port_t sin_port;    //16bit的端口号
    struct s_addr sin_addr; //IP地址
    char sin_zero[8];       //未曾使用
}
```
需要注意的是tcp、udp端口号在socket地址结构中，总是以<font color=#FF000>网络字节序(即高位字节存放到低位地址上)</font>来存储的