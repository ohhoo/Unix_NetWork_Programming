# Unix_NetWork_Programming
学习Unix网络编程的相关代码示例、学习笔记等文档

# 环境安装、配置
首先下载 [unpv13e.tar.gz](https://github.com/yzfedora/unpv13e/archive/refs/heads/master.zip) 解压后进入`unpv13e`文件夹中执行 `./configure` 
```
cd ./lib
make #编译lib文件夹中的文件

cd ../libfree
make #编译libfree文件夹中的文件
```
执行完上述操作后，返回`unpv13e`文件夹主目录下，发现生成了`libunp.a`文件
```
cp libunp.a /usr/lib/
cp libunp.a /usr/lib64

#将所需头文件放在对应文件夹下，unp为新建文件夹
cp config.h /usr/include/unp
cp ./key/unp.h /usr/include/unp
```
修改`/usr/include/unp/unp.h`第七行中的`#include "../config.h"` 为`#include "config.h"`

编译自带的例子查看配置是否生效，示例文件路径`unpv13e/intro/daytimetcpcli.c`，编译命令：
```
gcc daytimetcpcli.c -o cli -lunp
```
<br></br>
在编译过程中出现如下报错信息：
```
/usr/include/unp/unp.h:122:8: error redefinition of 'struct in_pktinfo'
```
这是因为in_pktinfo结构体重复定义导致的，修改方法为：在`/usr/include/unp/config.h`文件中添加
```
#define HAVE_STRUCT_IP_PKTINFO 1
```
并在`/usr/include/unp/unp.h`中将`in_pktinfo`结构体定义用`#ifndef HAVE_STRUCT_IP_PKTINFO #endif`包起来，防止重复定义出现即可。
<br></br>
还会出现`err_sys`以及`err_print`等函数未定义的错误出现，这是因为这些函数是单独写出来的，还需要重新下载配置，可以用exit、printf等函数替换，也可以找到这些函数的定义，将其单独写入一个文件并在代码中引用即可，这些函数的定义如下
```c
#include <errno.h> /* for definition of errno */
#include <stdarg.h> /* ISO C variable aruments */

static void err_doit(int, int, const char *, va_list);

/*
 * Nonfatal error related to a system call.
 * Print a message and return.
 */
void err_ret(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
}


/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}


/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void err_exit(int error, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, error, fmt, ap);
    va_end(ap);
    exit(1);
}


/*
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void err_dump(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    abort(); /* dump core and terminate */
    exit(1); /* shouldn't get here */
}


/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void err_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
}


/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void err_quit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}


/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[MAXLINE];
   vsnprintf(buf, MAXLINE, fmt, ap);
   if (errnoflag)
       snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s",
         strerror(error));
   strcat(buf, "\n");
   fflush(stdout); /* in case stdout and stderr are the same */
   fputs(buf, stderr);
   fflush(NULL); /* flushes all stdio output streams */
}
```
<font color=#ff000>同时注意在引用这个函数定义的文件时，该文件的引用应该在unp.h文件引用的后面</font>

**注意事项：**<br></br>
1、在对时间服务器示例代码进行编译时会报错提示`Write`、`Socket`、`Bind`、`Close`这几个函数无法找到引用，可以将这几个函数的首字母改为小写。这是因为大写格式的函数是包裹函数是unp.h作者自定义的，会包含一些错误提示信息，小写格式的函数实际进行监听、绑定等功能的库函数。<br></br>
2、编译成功后在运行时间服务器函数时需要加上`sudo`赋予`root`权限，因为时间服务器代码中会进行端口开启的操作，如果没有权限开启端口在会导致时间客户端函数在获取时间时出现`connect refuse`