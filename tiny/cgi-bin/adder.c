/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p, *p1, *p2; 
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  if((buf = getenv("QUERY_STRING")) != NULL){
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    // arg1에는 n1=123
    strcpy(arg2, p+1);
    // arg2에는 n2=123 들어 있음 
    p1 = strchr(arg1, '=');
    p2 = strchr(arg2, '=');
    strcpy(arg1, p1 + 1);
    strcpy(arg2, p2 + 1);

    //printf("출력해보기 %s %s", arg1, arg2);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sThe Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1+n2);
  sprintf(content, "%sThanks for visiting!\r\n",content );

  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}
/* $end adder */
