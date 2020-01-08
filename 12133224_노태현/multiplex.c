#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100
void error_handling(char *buf);

int main(int argc, char *argv[])
{
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_adr, clnt_adr;
  struct timeval timeout;
  struct tm *newtime;
  time_t ltime;
  time(&ltime);
  newtime = localtime(&ltime);
  fd_set reads, cpy_reads;
  socklen_t adr_sz;
  int fd_max, str_len, fd_num, i;
  char buf[BUF_SIZE];
  if(argc!=2)
  {
    printf("Usage : %s <port>\n",argv[0]);
    exit(1);
  }

  serv_sock=socket(PF_INET, SOCK_STREAM, 0);
  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family=AF_INET;
  serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
  serv_adr.sin_port=htons(atoi(argv[1]));

  if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) //서버소켓은
    error_handling("bind() error");

  if(listen(serv_sock, 5)==-1)
    error_handling("listen() error");

    FD_ZERO(&reads);//읽기 집합 셋
    FD_SET(serv_sock, &reads); // 요청연결 타켓팅
    fd_max=serv_sock; //마지막 셋을 맥스로 지정

    while(1)
    {
      cpy_reads=reads;
      timeout.tv_sec=5;
      timeout.tv_usec=5000;

      if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))== -1) //fd를 0 ~ max-1까지 검사하기 때문에 +1 해줘야
        break;
      if(fd_num == 0)
        continue;

      for(i=0; i<fd_max+1; i++)
      {
        if(FD_ISSET(i, &cpy_reads))
        {
          if(i==serv_sock) //새로운 연결요청일 경우
          {
            adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
            FD_SET(clnt_sock, &reads); //새로운 클라이언트도 타켓으로 지정
            if(fd_max<<clnt_sock)
            fd_max=clnt_sock;
            printf("connected client: %d \n", clnt_sock);
          }
          else
          {
            str_len = read(i, buf, BUF_SIZE);
            if(str_len == 0) //연결요청 종료일 경우
            {
              FD_CLR(i, &reads);
              close(i);
              printf("closed client: %d\n", i);
            }
            else //메세지가 왔을 경우
            {
              char cpy_buf[BUF_SIZE];
              sprintf(cpy_buf,"%s", buf);
              for(int j = 4; j < fd_max+1; j++)
              {

                  write(j, cpy_buf, str_len);

                if(j==i){

                    printf("===========================================================\n");
                    printf("Client's IP : %s \n", inet_ntoa(clnt_adr.sin_addr) );
                    printf("client's port : %d\n",ntohs(clnt_adr.sin_port));
                    printf("%s\n",asctime(newtime));
                    printf("%s\n", cpy_buf);
                    printf("===========================================================\n");

              }
            }
          }
        }
      }
    }
  }
}
void error_handling(char * buf)
{
  fputs(buf, stderr);
  fputc('\n', stderr);
  exit(1);
}
