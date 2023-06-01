#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <time.h>

#define BUF_SIZE 1024

int main(int argc, char **argv)
{

 bool debug = false;
 char* HOST = "iot.dslab.pub.ds.open-cloud.xyz";
 int PORT = 18080;
 char buf[BUF_SIZE];
 ///////////////////////////////////////////////////////////////////////////
 //Checking for errors in input from argv and receiving terminal variables//
 ///////////////////////////////////////////////////////////////////////////
 if(argc > 6)
 {
    printf("Error-Usage : ./main2 [--host HOST] [--port PORT] [--debug]\n");
    exit(-1);
 }
 if(argc > 1)
 {
    for(int i=1; i<argc; i++)
    {
      if(!strcmp(argv[i], "--host"))
      {
        HOST = argv[i+1];
        i++;
      }
      else if(!strcmp(argv[i], "--port"))
      {
        PORT = atoi(argv[i+1]);
        i++;
      }
      else if(!strcmp(argv[i], "--debug"))
        debug = true; 
      else
      {
        printf("Error-Usage : ./main2 [--host HOST] [--port PORT] [--debug]\n");
        exit(-1);   
      }
    }
 }
 /////////////////////////////////////////
 //Establishing connection to the server//
 /////////////////////////////////////////
 int sd = socket(AF_INET,SOCK_STREAM,0); 
 if (sd < 0) 
 {
  perror("socket");
  return -1;
 }

 struct sockaddr_in addr; 
 struct hostent *hostp;

 hostp = gethostbyname(HOST); 

 addr.sin_family = AF_INET;
 addr.sin_port = htons(PORT);
 bcopy(hostp->h_addr_list[0] , &addr.sin_addr, hostp->h_length);

 if (connect(sd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
 {
    perror("connect");
    return -1;
 }
 //////////////////////////////////////////////////////////////////////////////
 //From here on we use select to read data from the terminal or the server/////
 //According to the user input we send the appropriate requests to the server//
 //////////////////////////////////////////////////////////////////////////////
 fd_set read_set;
 int ndfs = sd;                                  //This is the first argument of select(), maximum file descriptor value
while(true)
{ 
    FD_ZERO(&read_set);                              //We initialize 
    FD_SET(0,&read_set);                             //Terminal
    FD_SET(sd,&read_set);                            //socket

    int retval = select(ndfs + 1, &read_set, NULL, NULL, NULL);
    if (retval < 0)
    {
        perror("select");
        return 1;
    }
    if (FD_ISSET(0, &read_set))                 //Data from terminal
    {
        if (fgets(buf, BUF_SIZE, stdin) == NULL)
        {
            perror("fgets");
            return -1;
        }
        if(!strcmp(buf,"exit\n"))
        {
            printf("Exiting...\n");
            close(sd);
            exit(0);
        }   
        if(!strcmp(buf,"help\n"))
        {
            printf("Type 'exit' to exit, 'get' to recover server data, 'N name surname reason' to request access to quarantine.\n");
            continue;
        }
        if(debug)
        {    
          int i = 0;
          while(buf[i] != '\n')                //We remove \n to print '%s' without changing line
            ++i;
          buf[i] = '\0';
          printf("[DEBUG] sent '%s'\n",buf);       
        }
       if(write(sd,&buf,strlen(buf) + 1) == -1)//writing to the server
       {
            perror("write");
            return -1;
       } 
    }  
    else if(FD_ISSET(sd, &read_set)) //We receive data from the server
    {
        char output[BUF_SIZE]; 
        int nbytes=read(sd,&output,BUF_SIZE-1);

        if( nbytes == -1 )
        {
            perror("read");
            return -1;
        }
        output[nbytes-1] = '\0';                   
        if(debug)
            printf("[DEBUG] read '%s'\n",output);
        if(!strcmp(output,"try again") || !strcmp(output,"invalid code"))
        {
            printf("%s\n",output);
            continue;
        }
        if(output[1] == ' ')      //If the second character is space we know the server sent a reply to
        {                         //a 'get' request as we have the format X YYY ZZZZ WWWWWWWWWW     
            int flag;
            int light;
            int temp;
            int timestamp;
            char* token;
            const char s[2] = " ";

            token = strtok(output, s);  //Here we separate the X YYY ZZZZ and WWWWWWWWWW strings to print them
            flag = atoi(token);

            token = strtok(NULL, s);
            light = atoi(token);

            token = strtok(NULL, s);
            temp = atoi(token);

            token = strtok(NULL, s);
            timestamp = atoi(token);
        
            char* message;
            double temperature = (double)temp/100;
            time_t senttime = timestamp;
            struct tm *info;
            time( &senttime );
            info = localtime( &senttime );

            switch (flag)   //Searching which flag we received
            {
                case 0:
                    message = "boot";
                    break;
                case 1:
                    message = "setup";
                    break;
                case 2:
                    message = "interval";
                    break;
                case 3:
                    message = "button";
                    break;
                case 4:
                    message = "motion";
                    break;
                default:
                    message = "unknown flag";
                    break;
            }
            printf("---------------------------\n");
            printf("Latest event:\n");
            printf("%s (%d)\n", message, flag);
            printf("Temperature is: %.2f\n",temperature);
            printf("Light level is: %d\n",light);
            printf("Timestamp is: %s", asctime(info));
            printf("---------------------------\n");
        }
        else if(output[0] == 'A' && output[1] == 'C' && output[2] == 'K') // 'ACK' reply
        {  
            printf("Response: '%s'\n",output);
        }
        else
            printf("Send verification code : '%s'\n",output);
    }
}
return 0;
}
