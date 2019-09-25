#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define MAX_ARGS      512
#define MAX_LENGTH    2048

int bgPs[100];
char* args[MAX_ARGS];
char input[MAX_LENGTH];
int quit;
int statusCode = 0;
int bgExitStatus = 0;
int signalNum = 0;
pid_t bgpid[100];
int nPs = 0;
int allowBg = 1;

char* replace_str(char* str, char* orig, char* rep);
void printStatusCode(int status);
void catchSigStop(int sigNum);

char* replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))
    return str;

  strncpy(buffer, str, p-str);
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

void printStatusCode(int status)
{
  if(WIFEXITED(status) == 1)
  {
    printf("exit value %d\n", WEXITSTATUS(status));
  }
  else
  {
    printf("terminated by signal %d\n", status);
  }
}

void catchSigStop(int sigNum)
{
	// If it's 1, set it to 0 and display a message reentrantly
	if (allowBg == 1) {
		char* Fgmessage = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, Fgmessage, 50);
		//fflush(stdout);
		allowBg = 0;
	}

	// If it's 0, set it to 1 and display a message reentrantly
	else {
		char* Bgmessage = "\nExiting foreground-only mode\n";
		write (STDOUT_FILENO, Bgmessage, 31);
		//fflush(stdout);
		allowBg = 1;
	}
}

int main()
{
  pid_t cpid;
  struct sigaction sa_sigint = {0};
	sa_sigint.sa_handler = SIG_IGN;
	sigfillset(&sa_sigint.sa_mask);
	sa_sigint.sa_flags = 0;
	sigaction(SIGINT, &sa_sigint, NULL);


  struct sigaction stopAction = {0};
  stopAction.sa_handler = catchSigStop;
  stopAction.sa_flags = 0;
  sigfillset(&(stopAction.sa_mask));
  sigaction(SIGTSTP, &stopAction, NULL);



  while(quit == 0)
  {
    int background = 0;
    fflush(stdin);
    fflush(stdout);
    printf(": ");
    char p[20];
    char* token;
    char buffer[50];
    int iter;
    char* inputFile = NULL;
    char* outputFile = NULL;
    int fileIn = -1;
    int fileOut = -1;
    int rd = 0;
    char* command;
    int rdI = 0;
    int rdO = 0;
    int comment = 0;

    fgets(input, 2048, stdin);
    //printf("%s", input);
    if(input[0] == '\n' || input[0] == '\0')
    {
      continue;
    }

    if(strstr(input, "#") != NULL)
    {
      continue;
    }

    if(strstr(input, "$$") != NULL)
    {
      sprintf(p, "%d", (int)getpid());
      char* p1;
      p1 = (char *) malloc((50) * sizeof(char));
      strcpy(p1, replace_str(input, "$$", p));
      strcpy(input, p1);
      free(p1);
    }

    if(strstr(input, "&") != NULL && allowBg == 1)
    {
      background = 1;
      int length = strlen(input);
      int tIndex;
      for(int i = 0; i < length; i++)
      {
        if(input[i] == '&')
        {
          tIndex = i;
        }
      }
      memmove(&input[tIndex], &input[tIndex + 1], strlen(input) - tIndex);
    }
    iter = 0;
    int numargs = 1;
    char* commandName;
    args[0] = strtok(input, " \n");
    commandName = args[0];
    //strcpy(commandName, args[0]);
    //printf("%s\n", args[0]);

    for(i = 1; i < MAX_ARGS; i++)
    {
      args[i] = strtok(NULL, " \n");
      if(args[i] != NULL && strcmp(args[i], "&") != 0)
      {
        numargs = numargs + 1;
      }
    }
    printf("%d\n", numargs);
    //args[numargs] = NULL;
    if(strcmp(args[0], "cd") == 0 && comment == 0)
    {
      if(numargs == 1)
      {

        char* home = getenv("HOME");
        chdir(home);
        printf("changed dir");
        continue;
      }
      else
      {
        chdir(args[1]);
        continue;
      }
    }

    else if(strcmp(args[0], "status") == 0 && comment != 1)
    {
      printStatusCode(statusCode);
      continue;
    }

    else if(strcmp(args[0], "exit") == 0 && comment != 1)
    {
      for(int i = 0; i < nPs; i++)
      {
        kill(bgPs[i], SIGKILL);
      }
      exit(0);
    }


    else
    {

      cpid = fork();
      switch(cpid)
      {
        case 0:
          printf("hello1");
          sa_sigint.sa_handler = SIG_DFL;
			    sigaction(SIGINT, &sa_sigint, NULL);
          if(background == 0) //not background
          {
            for(i = 0; i < numargs; i++)
            {
              printf("processing argument '%s'", args[i]);
              if(strcmp(args[i], "<") == 0)
              {
                strcpy(inputFile, args[i + 1]);
                fileIn = open(inputFile, O_RDONLY);
                rdI = 1;
                if(fileIn == -1)
                {
                  printf("cannot open %s for input\n", inputFile);
                  exit(1);
                }
                dup2(fileIn, 0);
                close(fileIn);
                //strcpy(command, args[0]);
                //execvp(args[0], args);
              }
              else if(strcmp(args[i], ">") == 0)
              {
                strcpy(outputFile, args[i + 1]);
                fileOut = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                rdO = 1;
                if(fileOut == -1)
                {
                  printf("cannot open %s", outputFile);
                  exit(1);
                }
                dup2(fileOut,1);
                close(fileOut);
              }
            }
            if(rd != 1) //no redirect
            {
              printf("hello");
              execvp(args[0], args);
            }
            else
            {
              printf("redirection");
              execvp(args[0], &args[0]);
            }
            printf("\"%s\" is not a valid command\n", args[0]);
            statusCode = 1;
            exit(1);

          }
          else //background
          {
            printf("%d", cpid);
            for(i = 0; i < numargs; i++)
            {
              if(strcmp(args[i], "<") == 0)
              {
                strcpy(inputFile, args[i + 1]);
                fileIn = open(inputFile, O_RDONLY);
                rdI = 1;
                if(fileIn == -1)
                {
                  printf("cannot open %s for input\n", inputFile);
                  exit(1);
                }
                dup2(fileIn, 0);
                close(fileIn);
                //strcpy(command, args[0]);
                //execvp(args[0], args);
              }
              else if(strcmp(args[i], ">") == 0)
              {
                strcpy(outputFile, args[i + 1]);
                fileOut = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                rdO = 1;
                if(fileOut == -1)
                {
                  printf("cannot open %s", outputFile);
                  exit(1);
                }
                dup2(fileOut,1);
                close(fileOut);
              }
            }
            if(rdI == 0)
            {
              fileIn = open("/dev/null", O_RDONLY);
              dup2(fileIn, 0);
              close(fileIn);
            }
            if(rdO == 0)
            {
              fileOut = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
              dup2(fileOut, 1);
              close(fileOut);
            }
            if(rdI == 1 || rdO == 1)
            {
              printf("hellobg");
              execvp(args[0], args);
            }
            else
            {
              printf("hellobg");
              execvp(args[0], args);
            }
            printf("\"%s\" is not a valid command\n", args[0]);
            statusCode = 1;
            exit(1);

          }

        case -1:
          perror("error with fork");
          statusCode = 1;
          break;

        default:
          if(background != 1 || allowBg == 0)
          {
            printf("executing foreground\n");
            waitpid(cpid, &statusCode, 0);
          }
          else
          {
            printf("background pid is %i\n", cpid);
            bgPs[nPs] = cpid;
            nPs = nPs + 1;
            waitpid(cpid, &statusCode, WNOHANG);
          }
          for(i = 0; i < nPs; i++)
          {
            bgPs[i] = waitpid(bgPs[i], &statusCode, WNOHANG);
            if(bgPs[i] != 0)
            {
              if (WIFEXITED(statusCode))
              {
                bgExitStatus = WEXITSTATUS(statusCode);
                printf("background pid %d is done: exit value %d.\n", bgPs[i], bgExitStatus);
                bgPs[i] = 0;
                nPs = nPs - 1;
              }
              else
              {
                bgExitStatus = WTERMSIG(statusCode);
                printf("background pid %d is done: terminated by signal %d\n", bgPs[i], bgExitStatus);
                bgPs[i] = 0;
                nPs = nPs - 1;
              }
            }
          }
      }
    }

    for(i = 0; i < MAX_ARGS; i++)
    {
      args[i] = NULL;
      //strcpy(args[i], "");
    }

  }
  return 0;
}
