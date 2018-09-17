#ifndef USER_HTTP_SERV_H
#define USER_HTTP_SERV_H
#else
#error You have already included an httpserv.h and it should not be included twice
#endif /* USER_HTTP_SERV_H */

#include "event2/http.h"
#include "event2/event.h"
#include "event2/buffer.h"
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

#include <map>  
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <string.h>
using namespace std;  

void HttpGenericCallback(struct evhttp_request* request, void* arg);

#define HTTP_REQ_GET   1
#define HTTP_REQ_POST  2
#define FWITE(responseWriter, fmt, args...) evbuffer_add_printf(responseWriter->evbuf,fmt, ##args)


class ResponseWriter{
public:
    struct evbuffer* evbuf;
public:
    ResponseWriter(struct evbuffer* buf){
        evbuf = buf;
    }
    void Write(const char *fmt, ...){
        //evbuffer_add_printf(evbuf, "Server response. urlkey is (%s)\n", url);
        va_list arg_ptr;//定义指向参数的指针
        int fmtLen = strlen(fmt);
        va_start(arg_ptr,fmt);//初始化指针arg_ptr，第二个实参为第一个可变参数的前一个固定参数
        for(int i = 0; i <= fmtLen - 1; ++i)
        {
            if(i <= fmtLen - 2 && fmt[i] == '%' && fmt[i + 1] == 's')
            {
                char* t = va_arg(arg_ptr,char*);//返回可变参数，类型为char*
                cout << t;
                i++;
            }
            else if(i <= fmtLen - 2 && fmt[i] == '%' && fmt[i + 1] == 'c')
            {
                //在C/C++环境中，当省略号对应的实参为bool/char/short时，将自动转换为 int进行传递
                char t = va_arg(arg_ptr,int);//返回可变参数，类型为int
                cout << t;
                i++;
            }
            else if(i <= fmtLen - 2 && fmt[i] == '%' && fmt[i + 1] == 'd')
            {
                int t = va_arg(arg_ptr,int);//返回可变参数，类型为int
                cout << t;
                i++;
            }
            else
                cout << fmt[i];
        }
        va_end(arg_ptr);
        //evbuffer_add_printf(evbuf, "Server response. urlkey is (%s)\n", url);
        evbuffer_add_vprintf(evbuf,fmt,arg_ptr);
        return;

    }
};


class PostData{
public:
    char* buf;
    map<string, string> items;     
public:
    PostData(){
        buf = NULL;
    }
    ~PostData(){
        if(buf != NULL)
            free(buf);
    }
    void Init(char* data){
        int len = strlen(data);
        buf = (char *) malloc(len + 1);

        memset(buf,0,len+1);
        strcpy(buf,data);
        
        int start = 0;
        printf("buf(%d):%s\n",len,buf);
        printf("buf:%s\n",buf);

        for(int i=0;i<=len;i++){
            if(buf[i] == '&' || buf[i] == '\0'){
                buf[i] = '\0';
                printf("%d : %s\n",i,&buf[start]);
                parserItem(buf,start,i);
                start = i+1;
            }
        }
        //printf("buf:%s\n",buf);
    }
  

    char* Get(string key){
        cout << "get-key:" << key <<std::endl;
        map<string, string>::iterator iter = items.find(key);
        if(iter != items.end()){
            cout <<"get-key(" << iter->first  << ") val " << iter->second << std::endl;
            return (char*)iter->second.data();
        }
        return NULL;
    }
private:
    void parserItem(char* buf,int start,int end){
        char* pData = NULL;
        for(int i=start;i<end;i++){
            if (buf[i] == '='){
                pData = &buf[i+1];
                buf[i] = '\0';
                break;
            }
        }

        if(pData != NULL){
            items.insert(pair<char*, char*>(&buf[start], pData));  
        }

        printf("key=[%s],val=[%s]\n",&buf[start],pData);
    }

};

class Request{
private:
    struct evbuffer* evbuf;
    PostData postData;
public:
    struct evhttp_request* request;
    
    struct evkeyvalq http_query_post;
    char *post_data;
    int requestType;
public:
    Request(struct evhttp_request* req,struct evbuffer* buf){
        request = req;
        evbuf = buf;
        requestType = 0;
        post_data = NULL;
    }
    ~Request(){
        if(post_data != NULL)
            free(post_data); 
    }
public:
    void init(){
        //char decode_post_uri[1024] = {0};
        int buffer_data_len = evbuffer_get_length(request->input_buffer);
        post_data = (char *) malloc(buffer_data_len + 1);
        memset(post_data, '\0', buffer_data_len + 1);
        memcpy(post_data, evbuffer_pullup(request->input_buffer,-1), buffer_data_len);
        
        postData.Init(post_data);
        //sprintf(decode_post_uri,"/?%s",post_data); 
        //evhttp_parse_query(decode_post_uri, &http_query_post);
        //const char *http_post_name = evhttp_find_header(&http_query_post, "host");
 
        //printf("init post_data(%d)=%s\n",buffer_data_len,decode_post_uri);
        //printf("init host=%s\n",http_post_name);
    }

    char* Get(string key){
        //return evhttp_find_header(&http_query_post, key);
        return postData.Get(key);
    }
};
 
typedef int (*HttpHandler)(Request*,ResponseWriter*);

class HttpServer{
public:
    struct event_base* base;
    struct evhttp* http;
    static map<string,HttpHandler> routeMap;        

public:
    HttpServer(){
        base = NULL;
        http = NULL;
    }
    bool ListenAndServe(const char* address, unsigned short port){
        base = event_base_new();
        if (!base)
        {
            printf("create event_base failed!\n");
            return false;
        }
 
        http = evhttp_new(base);
        if (!http)
        {
            printf("create evhttp failed!\n");
            return false;
        }

        if (evhttp_bind_socket(http, "0.0.0.0", port) != 0)
        {
            printf("bind socket failed! port:%d\n", port);
            return false;
        }

        evhttp_set_gencb(http, HttpServer::HttpCallback, NULL);

        event_base_dispatch(base);
        return true;
    }

    void HandleFunc(const char* pattern, HttpHandler handler){
        routeMap.insert(pair<string, HttpHandler>(pattern, handler));  
    }

    static int callHandler(char* url,Request* req,ResponseWriter* resp){
        char action[255];
        int i;
        memset(action, '\0', 255);

        for(i=0;i<200;i++){
            if(url[i] != '\0')
                action[i] = url[i];
        }

        printf("action:%s\n",action);

        map<string,HttpHandler>::iterator iter = routeMap.find(action);
        if(iter != routeMap.end()){
            iter->second(req,resp);
            return HTTP_OK;
        }
        return HTTP_NOTFOUND;
    }
 
    static void HttpCallback(struct evhttp_request* request, void* arg){
        const struct evhttp_uri* evhttp_uri = evhttp_request_get_evhttp_uri(request);
        char url[8192];
        evhttp_uri_join(const_cast<struct evhttp_uri*>(evhttp_uri), url, 8192);

        printf("accept request url:%s\n", url);

        struct evbuffer* evbuf = evbuffer_new();
        if (!evbuf)
        {
            printf("create evbuffer failed!\n");
            return ;
        }

        //evbuffer_add_printf(evbuf, "Server response. urlkey is (%s)\n", url);
        
        Request req(request,evbuf);
        req.init();

        ResponseWriter resp(evbuf);
        if (callHandler(url,&req,&resp) == HTTP_OK){
            //evbuffer_add_printf(evbuf, "handler 200. Your request url is %s\n", url);
            evhttp_send_reply(request, HTTP_OK, "OK", evbuf);
        }else{
            //evbuffer_add_printf(evbuf, "handler 404. Your request url is %s\n", url);
            evhttp_send_reply(request, HTTP_NOTFOUND, "HTTP_NOTFOUND", evbuf);
        }
        evbuffer_free(evbuf);
    }
};

map<string,HttpHandler> HttpServer::routeMap = map<string,HttpHandler>();



