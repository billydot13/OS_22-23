#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>

int gate;
char state;
time_t start_time;

void sig_handlers(int signal)
{
  if(signal == SIGPIPE)
  {//flip taste
        if (state == 't')
        {
          state = 'f';
          printf("[GATE=%d/PID=%d/TIME=%li s] The gates are closed!\n",gate,getpid(),(time(NULL) - start_time));
        }
        else
        {
          state = 't';
          printf("[GATE=%d/PID=%d/TIME=%li s] The gates are open!\n",gate,getpid(),(time(NULL)- start_time));
        }

    }
    else if(signal == SIGUSR2)
    {//print to state
      if (state == 't')
      {
        printf("[GATE=%d/PID=%d/TIME=%li s] The gates are open!\n",gate,getpid(),(time(NULL) - start_time));
      }
      else
      {
        printf("[GATE=%d/PID=%d/TIME=%li s] The gates are closed!\n",gate,getpid(),(time(NULL) - start_time));
      }
    }
    else if(signal == SIGALRM)
    {
      alarm(15);
      if (state == 't')
      {
        printf("[GATE=%d/PID=%d/TIME=%d s] The gates are open!\n",gate,getpid(),(int)(time(NULL) - start_time));
      }
      else
      {
        printf("[GATE=%d/PID=%d/TIME=%d s] The gates are closed!\n",gate,getpid(),(int)(time(NULL) - start_time));
      }
    }
    else if(signal == SIGTERM)
    {
      //termatizoun
      exit(0);
    }
}
void set_signal_action(void)
{
  struct sigaction	action;
  action.sa_handler = &sig_handlers;
  action.sa_flags = SA_RESTART;
  sigaction(SIGPIPE, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);
  sigaction(SIGALRM, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

}

int main(int argc, char **argv)
{
  set_signal_action();
  alarm(15);
  gate = atoi(argv[2]);//typecasting
  state = argv[1][0];
  if (state == 't')
  {
    printf("[GATE=%d/PID=%d/TIME=0 s] The gates are open!\n",gate,getpid());
  }
  else
  {
    printf("[GATE=%d/PID=%d/TIME=0 s] The gates are closed!\n",gate,getpid());
  }
  start_time = time(NULL);
  while (1);
  return 0;
}
