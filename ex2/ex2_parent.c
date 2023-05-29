#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include<stdbool.h>
#include <errno.h>

void check_neg(int ret, const char *msg) {
  if (ret < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

int p_id[50];
int number_of_processes ;
char states[50];
bool termination = false;
int find_child (int dead_pid) 
{ 
  for(int i=0; i<number_of_processes; ++i)
  {
    if(p_id[i] == dead_pid)
      return i;
  }
  return -1;
}

void sig_handlers(int signal)
{
  if(signal == SIGPIPE)//valame to shma SIGPIPE giati to SIGUSR1 den leitourgouse
  {
    //flip the state
    for (int i = 0; i < number_of_processes; i++)
    {
      kill(p_id[i],SIGPIPE);
    }
        
  }
  else if(signal == SIGUSR2)
  {//prints the state
    for (int i = 0; i < number_of_processes; i++)
    {
      kill(p_id[i],SIGUSR2);
    }
  }
  else if(signal == SIGTERM)
  {//terminates the children processes

    termination = true;
    int number = number_of_processes;
    int status;
    for (int i = 0; i < number_of_processes; i++)
    {
      printf("[PARENT/PID = %d] Waiting for %d children to exit\n",getpid(),number);
      kill(p_id[i],SIGTERM);
      --number;
      if(wait(&status) == -1)//perimenoume na teleiwsei to process pou molis kaname kill me to sigterm
      {
        printf("There are no child processes to wait for\n");
      }
      printf("[PARENT/PID = %d] Child with PID %d exited with status code %d\n",getpid(), p_id[i],WEXITSTATUS(status));
    }
    printf("[PARENT/PID = %d] All children exited,terminating as well\n",getpid());
    exit(0);
  }
  else if(signal == SIGCHLD)
  {
    if (!termination)//otan steiloume sigterm ston parent den theloume na 3anagennhsei ta paidia pou skotwnei
    {
        int status;
        pid_t result = waitpid(-1,&status,WUNTRACED);//wuntraced molis vrei paidi pou egine stop h termatisthke epistrefei to pid tou.. kai to -1 psaxnei ola ta paidia
        int i = find_child(result);
        if (result > 0 )
        { 
          if(WIFEXITED(status) || WIFSIGNALED(status))
          {
            printf("[PARENT/PID=%d] Child %d with PID=%d exited\n",getpid(),i,p_id[i]); 
            pid_t p = fork();
            check_neg(p, "fork");
            if (p == 0)
            {
              printf("[PARENT/PID = %d] Created child %d (PID = %i) and initial state %c\n",getppid(),i,getpid(), states[i]);
              char ar[16];
              sprintf(ar,"%d",i);
              char *const argv[] = {"./child",&states[i],ar, NULL};
              int status = execv("./child", argv);
              /* on success, execution will never reach this line */
              check_neg(status, "Failed to create child");
            }
            else if (p > 0)
            {
              p_id[i] = p;
            }
            else
              perror("error");
                
          }
          else if(WIFSTOPPED(status))
          {
            printf("Child %i stopped , resuming...\n",i);
            kill(p_id[i],SIGCONT);
          }
          else
          {
            printf("something went wrong in waitpid\n");
          }
       }
        else if(result == -1)//crashare
        {
          if (errno == ECHILD) {
                  printf("Error: no child processes to wait for\n");
              }
          else if (errno == EINTR) {
                  printf("Error: waitpid() interrupted by a signal\n");
          } 
          else {
                  perror("Error calling waitpid()");
                  exit(EXIT_FAILURE);
          }          
        }
      }
  }
}
void set_signal_action(void)
{
	struct sigaction	act;
	act.sa_handler = &sig_handlers;
  act.sa_flags = SA_RESTART;//meta thn prwth ektelesh oi handlers ginontan default opote me ayton ton tropo statheropoiountai
  sigaction(SIGPIPE, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGCHLD, &act, NULL);
}

int main(int argc, char **argv) {
  if(argc != 2)
  {
    printf("Usage:./a.out only strings of f or t \n");
    return 1;
  }
  for(int i=0; i<strlen(argv[1]); ++i)
  {
    if(argv[1][i] != 'f' && argv[1][i] != 't')
    {
      printf("Please use a string of characters f or t as input\n");
      exit(0);
    }
  }
  number_of_processes = strlen(argv[1]);
  set_signal_action();
  for (int j = 0; j < number_of_processes; j++)
  {
    pid_t p = fork();
    check_neg(p, "fork");
    states[j] = argv[1][j];
    if (p == 0)
    {
      printf("[PARENT/PID = %d] Created child %d (PID = %i) and initial state %c\n",getppid(),j,getpid(), states[j]);
      char ar[16];
      sprintf(ar,"%d",j);//typecasting
      char *const argv[] = {"./child",&states[j],ar, NULL};
      int status = execv("./child", argv);
      /* on success, execution will never reach this line */
      check_neg(status, "Failed to create child");
    }
    else if (p > 0)
    {
      p_id[j] = p;
    }
    else
      perror("error");
  }
 while (1);
 
  return 0;
}
