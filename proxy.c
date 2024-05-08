/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"
#include <stdlib.h>

#define MAX_OBJECT_SIZE 102400

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *hostname, char *port, char *path);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3)Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int clientfd)
{
  int serverfd, connfd;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], request_buf[MAXLINE], response_buf[MAX_OBJECT_SIZE];
  char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];
  rio_t rio, responseRio;

  Rio_readinitb(&rio, clientfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s\r\n", method, uri);

  parse_uri(uri, hostname, port, path);


  sprintf(request_buf, "%s /%s %s\r\n", method, path, "HTTP/1.1"); // 요청 라인 요청 버퍼에 추가
  printf("%s\n", request_buf);
  sprintf(request_buf, "%sConnection: close\r\n", request_buf);       // Connection 헤더 추가
  sprintf(request_buf, "%sProxy-Connection: close\r\n", request_buf); // Proxy-Connection 헤더 추가
  sprintf(request_buf, "%s%s\r\n", request_buf, user_agent_hdr);      // User-Agent 헤더 추가

  // 요청 메서드가 GET 인지 확인
  if (strcasecmp(method, "GET"))
  {
    // GET이 아닐 때 요청이 메서드가 HEAD인지 확인
    if (strcasecmp(method, "HEAD"))
    {
      clienterror(clientfd, method, "501", "Not implemented", "Tiny does not implment this method");
      return;
    }
  }


  // 프록시 서버와 엔드 서버 연결
  //serverfd = Open_clientfd("localhost", "8080");
  serverfd = Open_clientfd(hostname, port);
  printf("%s\n", request_buf);

  // 서버에 요청 전송 
  Rio_writen(serverfd, request_buf, strlen(request_buf));
  Rio_readinitb(&responseRio, serverfd);

  ssize_t n;
  
  while ((n = Rio_readlineb(&responseRio, response_buf, MAX_OBJECT_SIZE)) > 0)
  {
    Rio_writen(clientfd, response_buf, n);
    if(!strcmp(response_buf, "\r\n")){
      break;
    }
  }
  while((n = Rio_readlineb(&responseRio, response_buf, MAX_OBJECT_SIZE)) > 0){
    Rio_writen(clientfd, response_buf, n);
  }
  Close(serverfd);
}

//URI 값을 이용하여 hostname, port, path를 파싱하는 함수
int parse_uri(char *uri, char *hostname, char *port, char *path)
{

  // http:// 형태라면 호스트 시작 주소를 반환, /hello 형태이면 uri 시작 주소 반환
  char *hostname_ptr = strstr(uri, "//") != NULL ? strstr(uri, "//") + 2 : uri + 1;

  // 포트가 있다면 포트의 시작 주소 이전을 가리킴
  char *port_ptr = strstr(hostname_ptr, ":");
  // path가 있는 경우 패스의 시작 주소를 이전을 가리킴
  char *path_ptr = strstr(hostname_ptr, "/");

  if (path_ptr > 0)
  {
    // 포트의 끝을 표현하기 위함
    *path_ptr = '\0';
    strcpy(path, path_ptr + 1); // 경로만 path에 저장
  }

  if (port_ptr > 0)
  {
    // hostname의 끝은 표현하기 위함
    *port_ptr = '\0';
    strcpy(port, port_ptr + 1);
  }
  strcpy(hostname, hostname_ptr);
  
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXLINE];

  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  sprintf(buf, "HTTP/1.1 %s %s \r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // line:netp:tiny:doit
    Close(connfd); // line:netp:tiny:close
  }
}