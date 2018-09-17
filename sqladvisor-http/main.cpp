#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#include "sqladvisor.h"
#include "event2/http.h"
#include "httpserv.h"
#include "inireader.h"

#define ASSERT_VAR(name) char* name =req->Get(#name);if(name == NULL){FWITE(w,"param error:%s",#name);return 1;}


void my_daemon() {
    int pid, fd;

    // 1.转变为后台进程
    if ((pid = fork()) == -1) exit(1);
    if (pid != 0) exit(0); // 父进程（前台进程）退出

    // 2.离开原先的进程组，会话
    if (setsid() == -1) exit(1); // 开启一个新会话

    // 3.禁止再次打开控制终端
    if ((pid = fork()) == -1) exit(1);
    if (pid != 0) exit(0); // 父进程（会话领头进程）退出

    // 4.关闭打开的文件描述符，避免浪费系统资源
    for (int i = 0; i < NOFILE; i++)
        close(i);

    // 5.改变当前的工作目录，避免卸载不了文件系统
    if (chdir("/") == -1) exit(1);

    // 6.重设文件掩码，防止某些属性被父进程屏蔽
    if (umask(0) == -1) exit(1);

    // 7.重定向标准输入，输出，错误流，因为守护进程没有控制终端
    if ((fd = open("/dev/null", O_RDWR)) == -1) exit(1); // 打开一个指向/dev/null的文件描述符
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);

    // 8.本守护进程的子进程若不需要返回信息，那么交给init进程回收，避免产生僵尸进程
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) exit(1);
}

int actionTest(Request* req,ResponseWriter* w){

    ASSERT_VAR(host);
    ASSERT_VAR(port);
    ASSERT_VAR(user);
    ASSERT_VAR(password);
    ASSERT_VAR(db);
    ASSERT_VAR(sql);

    char* sqlList[2] = {sql,NULL};
    printf("host=%s\n",host);
    printf("port=%s\n",port);
    printf("user=%s\n",user);
    printf("password=%s\n",password);
    printf("db=%s\n",db);
    printf("sql=%s\n",sql);
  
    SqlAdvisor advisor(host,atoi(port),user, password,db,sqlList);
    advisor.run();

    for ( int ix = 0; ix < advisor.infoList.size(); ++ix ){
       FWITE(w,"%03d ### %s",ix + 1,advisor.infoList[ix].c_str()); 
    }
    
    //FWITE(w,"actionTest11 %s\n","hello22"); 
    return 0;
}

int main(int argc, char **argv) {
    IniReader reader;
    reader.parseIniFile("etc/config.ini");

    char* ip = reader.getConfigStr("listen","0.0.0.0");
    int port = reader.getConfigInt("port",12641);
    char* action = reader.getConfigStr("action","sqlcheck");
    int daemon = reader.getConfigInt("daemon",0);

    printf("action:(%s)\n",action);
    printf("listenip:(%s)\n",ip);
    printf("port:(%d)\n",port);
    printf("daemon:(%d)\n",daemon);

    if(daemon != 0){
        //daemon(1,0);
        my_daemon();
    }

    HttpServer httpServ;
    httpServ.HandleFunc(action,actionTest);

    httpServ.ListenAndServe(ip, port);
    return 0;
}

