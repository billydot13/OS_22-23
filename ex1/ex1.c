#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
 struct stat file;
  if(argc != 2){
   printf("Usage: ./a.out filename\n");
   return 1;
  }
  else{
   if(strcmp(argv[1], "--help") == 0){
    printf("Usage: ./a.out filename\n");
    return 0;
   }
   else if(stat(argv[1], &file) == 0){
        printf("Error: %s already exists\n",argv[1]);
        return 1;
    }
  }
 int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY | O_RDONLY , 0644);
    //στο argv[1] , η δευτερη παραμετρος καθοριζει πως θα προσπελασουμε το αρχειο, η παραμετρος 0644 : (owning) User: read & write | Group: read | Other: read
    //https://stackoverflow.com/questions/18415904/what-does-mode-t-0644-mean
 if (fd == -1) {
         perror("open");
         return 1;
 }
 int status;
 pid_t child ;

  child = fork();
  if(child<0)
  {
       printf("Error in generating child program");
       return 1;
  }
  if(child==0){
      pid_t childid = getpid();
      pid_t parentid = getppid();
      char bufchild[50];
      sprintf(bufchild,"[CHILD] getpid()= %d,getppid()= %d\n",childid,parentid);

      if (write(fd, bufchild, strlen(bufchild)) < strlen(bufchild)) {
            perror("write");
          return 1;
       }
        exit(0);
    }
   else {
       wait(&status);
       pid_t childidd = getpid();
       pid_t parentidd = getppid();
       char bufparent[50];
       sprintf(bufparent,"[PARENT] getpid()= %d,getppid()= %d\n",childidd,parentidd);
       if(write(fd, bufparent, strlen(bufparent)) < strlen(bufparent)) {
            perror("write");
            return 1;
        }
        close(fd);
        if (close < 0)
        {
            perror("close error");
            return 1;
        }
        exit(0);
    }
  return 0;
}
