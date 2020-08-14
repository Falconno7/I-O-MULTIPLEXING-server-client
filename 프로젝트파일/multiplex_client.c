#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
#define NAME_SIZE 20

void error_handling(char *message);
void read_routine(int sock, char* message);
void write_routine(int sock, char *message);
char message[BUF_SIZE];
char name[NAME_SIZE] = "[NULL]";

int main(int argc, char *argv[])
{
  char message[BUF_SIZE];
  struct timeval timeout;
  int sock;
  int fd_num;
  struct sockaddr_in serv_adr;
  fd_set reads, cpy_reads;
  int fd_max;

  if (argc != 4){
    printf("Usage : %s <IP> <port> \n", argv[0]);
    exit(1);
  }
  sprintf(name, "[%s] : ", argv[3]);

  sock = socket(PF_INET, SOCK_STREAM, 0); //소켓생성, 이상황에서는 서버,클라이언트로 나눠지지 않음.
  if(sock == -1)
    error_handling("socket() error");

  memset(&serv_adr, 0, sizeof(serv_adr)); //서버 각주와 동일..
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = inet_addr(argv[1]);//입력인자에서 ip주소를 변수로 가져옴.
  serv_adr.sin_port = htons(atoi(argv[2])); //입력인자에서 포트번호를 변수로 가져옴.

  if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) //연결요청
    error_handling("connect() error!");
  else
    fputs("connected..............\n", stdout);

  FD_ZERO(&reads);
  FD_SET(STDIN_FILENO, &reads);
  FD_SET(sock, &reads);
  fd_max = sock;


  while(1)
  {
    cpy_reads = reads;
    timeout.tv_sec=5;
    timeout.tv_usec=5000;
    if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1)
      break;
    if(fd_num==0)
      continue;
    else{
      if(FD_ISSET(STDIN_FILENO, &reads)){
        write_routine(sock, message);
      }

      else if(FD_ISSET(sock, &reads)){
        read_routine(sock, message);
      }
    }

  }
  return 0;
}





void read_routine(int sock, char* message)
{
  char total_msg[NAME_SIZE + BUF_SIZE];
  while(1)
  {
    int str_len = read(sock, total_msg, NAME_SIZE+BUF_SIZE);
    if(str_len==0)
      return;
    total_msg[str_len]=0;
    fputs(total_msg, stdout);
    return;
  }
}

void write_routine(int sock, char *message)
{ char total_msg[NAME_SIZE+BUF_SIZE];
  while(1)
  {
    fgets(message, BUF_SIZE, stdin); //메세지 읽음.
    if(!strcmp(message,"q\n") || !strcmp(message, "Q\n")){ //q 혹은 Q입력시 종료
      printf("Q- 종료 -");
      shutdown(sock, SHUT_WR);
    }
    else{
      sprintf(total_msg," %s %s",name, message);
      write(sock, total_msg, strlen(total_msg));
    }
      return;
    }
  }

void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
