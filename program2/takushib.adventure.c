// Program2, Blaise Takushi CS344, OSUID: 932347942, takushib@oregonstate.edu
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>

//Initialize Mutex
pthread_mutex_t mutex1;

//Struct for room
struct Room
{
  char name[20];
  char roomType[20];
  int numberConnections;
  char connections[6][15];
};

//Hardcoded room names and room types
char* roomNames[10] = {"Death", "Life", "Purge", "Horror", "Lights", "Cowards", "Winners", "Losers", "Midnight", "Weirdos"};
char* types[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};

char* openFolder();
char* getFirstRoom();
void gameLoop();
void switchRoom(char* roomName, struct Room* room1);
void printTime();
void thread();
void* timeFile();

//Function to open correct rooms directory and return folder name (string)
char* openFolder()
{
  char targetDir[40] = "takushib.rooms.";
  static char newDir[100];
  int newestDirTime = -1;
  memset(newDir, '\0', sizeof(newDir));
  FILE* filep;
  DIR* dirToCheck;
  struct dirent *fileInDir;
  struct stat dirStats;
  //Open current directory
  dirToCheck = opendir(".");
  //Get the most recent directory
  if (dirToCheck > 0)
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL)
    {
      if (strstr(fileInDir->d_name, targetDir) != NULL)
      {
        stat(fileInDir->d_name, &dirStats);

        if ((int)dirStats.st_mtime > newestDirTime)
        {
          newestDirTime = (int)dirStats.st_mtime;
          memset(newDir, '\0', sizeof(newDir));
          strcpy(newDir, fileInDir->d_name);
        }
      }
    }
  }
  closedir(dirToCheck);
  // return the folder name
  return newDir;
}

//function that returns the name of the Start room
char* getFirstRoom()
{
  struct dirent* search;
  DIR* currentFolder;
  char currentLine[75];
  char fileName[50];
  char* folderName = openFolder();
  //Variables to hold the sscanf results
  char stringCheck1[20], stringCheck2[20], stringCheck3[20];
  static char startRoom[50];
  FILE* fileP;
  if((currentFolder = opendir(folderName)) == NULL)
  {
    perror("Unable to find folder");
    return "";
  }
  else
  {
    //While the current folder isn't empty, search through each room file
    while((search = readdir(currentFolder)) != NULL)
    {
      int i = 0;
      for(i = 0; i < 10; i++)
      {
        //if the name of the file is in the hardcoded list, continue
        if(strcmp(search -> d_name, roomNames[i]))
        {
          sprintf(fileName, "./%s/%s", folderName, search->d_name);
          //open the room file
          fileP = fopen(fileName, "r");
          //iterate through each line until the last one which holds the room type
          while(fgets(currentLine, 75, fileP) != NULL)
          {
            sscanf(currentLine, "%s %s %s", stringCheck1, stringCheck2, stringCheck3);
          }
          //if the room type is start room, save it to a variable
          if(!strcmp(stringCheck3, "START_ROOM"))
          {
            strcpy(startRoom, search -> d_name);
            break;
          }
        }
      }
    }
  closedir(currentFolder);
  return startRoom;
  }
}

//function that takes a room and sets all it's field to the room it's switching to
void switchRoom(char* roomName, struct Room* room1)
{
  FILE* fileP;
  char fileName[50];
  char Folder[50];
  int i;
  char stringCheck1[20], stringCheck2[20], stringCheck3[20];
  //set all the connections to nothing
  for(i = 0; i < room1->numberConnections; i++)
  {
    strcpy(room1->connections[i],"");
  }
  //reset the number of connections so that it can be built up later
  room1->numberConnections = 0;
  sprintf(Folder, "%s", openFolder());
  //build up file path string
  sprintf(fileName, "./%s/%s", Folder, roomName);
  fileP = fopen(fileName, "r");
  if(fileP == NULL)
  {
    perror("Could not open file");
    return;
  }
  char currentLine[75];
  fgets(currentLine, 75, fileP);
  int n = sscanf(currentLine, "%s %s %s", stringCheck1, stringCheck2, stringCheck3);
  strcpy(room1->name, stringCheck3);
  //while in the file, scan each line and set connections and types
  while(fgets(currentLine, 75, fileP) != NULL)
  {
    sscanf(currentLine, "%s %s %s", stringCheck1, stringCheck2, stringCheck3);
    if(strcmp(stringCheck1,"CONNECTION") == 0)
    {
      //set connections and increment count
      strcpy(room1->connections[room1->numberConnections], stringCheck3);
      room1->numberConnections = room1->numberConnections + 1;
    }
    else if(strcmp(stringCheck1,"ROOM") == 0 && strcmp(stringCheck2,"TYPE:") == 0)
    {
      //set type
      strcpy(room1->roomType, stringCheck3);
    }
  }
  return;
}

//gameLoop function that runs the actual game.
void gameLoop()
{
  char* firstRoom;
  //variable to store the path of the user, set number of steps to 0
  char userPath[50][15];
  int numberSteps = 0;
  int finished = 0;
  char userInput[50];
  //if for some reason the first room is null, exit the game.
  if (strcmp(getFirstRoom(), "") == 0)
  {
    exit(1);
  }
  //get the name of the first room
  firstRoom = getFirstRoom();
  struct Room* room = malloc(sizeof(struct Room));
  //build room struct for the first room
  switchRoom(firstRoom, room);
  int i;
  do
  {
    if(strcmp(userInput, "time") != 0)
    {
      //print current location and connections
      printf("CURRENT LOCATION: %s\n", room->name);
      printf("POSSIBLE CONNECTIONS: %s", room->connections[0]);
      i = 1;
      for(i = 1; i < room->numberConnections; i++)
      {
        printf(", %s", room->connections[i]);
      }
      printf(".\n");
    }
    printf("WHERE TO >");
    //get user input and store in variable
    scanf("%s", userInput);
    //if the user input is time, start second thread and print the time
    if(strcmp(userInput, "time") == 0)
    {
      //start thread and print the time
      thread();
      printTime();
    }
    else
    {
      int checker = 0;
      i = 0;
      //check if user input is a valid connection
      for(i = 0; i < room->numberConnections; i++)
      {
        if(strcmp(userInput,room->connections[i]) == 0)
        {
          checker = 1;
        }
      }
      //if the user input isn't valid, print error statement
      if(checker != 1)
      {
        printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      }
      else
      {
        //if the input is valid, change the room struct and print fields
        switchRoom(userInput, room);
        strcpy(userPath[numberSteps], userInput);
        numberSteps = numberSteps + 1;
        printf("\n");
        if(strcmp(room->roomType,"END_ROOM") == 0)
        {
          finished = 1;
          free(room);
        }
      }
    }

  } while(finished == 0);
  //print congratulatory message
  printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", numberSteps);
  int k;
  //print user path
  for(k = 0; k < numberSteps; k++)
  {
    printf("%s\n", userPath[k]);
  }
}

//printTime function that opens the time file and prints the time
void printTime()
{
  char timeBuffer[50];
  char time1[20], time2[20], time3[20], time4[20], time5[20];
  FILE* currentTimeFile;
  currentTimeFile = fopen("./currentTime.txt", "r");
  //print error if time file is empty
  if(currentTimeFile == NULL)
  {
    perror("Unable to open time File");
  }
  else
  {
    fgets(timeBuffer, 75, currentTimeFile);
    //scan the line and place into char array
    sscanf(timeBuffer, "%s %s %s %s %s", time1, time2, time3, time4, time5);
    //if the first number is 0, set it to a space as shown in the instructions
    if(time1[0] == '0')
    {
      time1[0] = ' ';
    }
    //change the AM and PM to lowercase as shown in instructions
    time1[5] = tolower(time1[5]);
    time1[6] = tolower(time1[6]);
    //print time to terminal
    printf("\n%s %s %s %s %s\n\n", time1, time2, time3, time4, time5);
    fclose(currentTimeFile);
  }
}

//timeFile function that creates/updates time file with the current time
void* timeFile()
{
  time_t t = time(0);
  struct tm* info;
  //get local time
  info = localtime(&t);
  FILE* currentTimeFile;
  currentTimeFile = fopen("currentTime.txt", "w+");
  char timeBuffer[50];
  //format the time to match the string shown in the instructions
  strftime (timeBuffer, sizeof(timeBuffer), "%I:%M%p, %A, %B %d, %Y", info);
  fputs(timeBuffer, currentTimeFile);
  fclose(currentTimeFile);
  return "";
}

//thread function that initializes a new thread
void thread()
{
  int result;
  pthread_t thread1;
  pthread_mutex_init(&mutex1, NULL);
  //lock the first thread
  pthread_mutex_lock(&mutex1);
  //create new thread that calls the timeFile function
  result = pthread_create(&thread1, NULL, timeFile, NULL);
  if(result != 0)
  {
    perror("Could not create thread");
  }
  //unlock main thread
  pthread_mutex_unlock(&mutex1);
  //destroy thread
  pthread_mutex_destroy(&mutex1);
  sleep(1);
}
int main()
{
  //run game loop function until user wins
  gameLoop();
  return 0;
}
