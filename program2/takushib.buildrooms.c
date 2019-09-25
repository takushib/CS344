// Program2, Blaise Takushi CS344, OSUID: 932347942, takushib@oregonstate.edu
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

//hardcoded room names and types
char* roomNames[10] = {"Death", "Life", "Purge", "Horror", "Lights", "Cowards", "Winners", "Losers", "Midnight", "Weirdos"};
char* types[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};

//room struct
struct Room
{
  char* name;
  char* roomType;
  int numberConnections;
  //connections is an array of ints
  int connections[6];
  int canAddConnection;
};

char* makeFolder();
void makeRooms(struct Room roomList[7]);
void makeConnections(int currentRoom, struct Room roomlist[7]);
void makeFiles(struct Room roomList[7], char* folder);

int isGraphFull(struct Room roomList2[7]);
int canAddConnectionFrom(struct Room room1);
void addRandomConnection(struct Room roomList3[7]);
int getRandomRoom();
int connectionAlreadyExists(struct Room roomlist3[7], int room1, int room2);
void connectRoom(struct Room rooms[7], int id1, int id2);
int isSameRoom(struct Room room1, struct Room room2);

//makeFolder function that creates room folder with process id, and returns the string
char* makeFolder()
{
  static char folderName[50];
  //build up folder name string
  sprintf(folderName, "takushib.rooms.%d", (int)getpid());
  if (mkdir(folderName, 0700) != 0)
  {
    perror("Unable to make directory.\n");
    return "";
  }
  return folderName;
}

//function that checks if all rooms have enough connections and returns 0 if false and 1 if true.
int isGraphFull(struct Room roomlist2[7])
{
  int graphFull = 1;
  int i;
  //for each room in a list, check if it has enough connections
  for(i = 0; i < 7; i++)
  {
    if(roomlist2[i].numberConnections < 3)
    {
      graphFull = 0;
    }
  }
  return graphFull;
}

//function that checks if the room can add another connections and returns 0 if false and 1 if true
int canAddConnectionFrom(struct Room room1)
{
  int result = 1;
  //if the room's number of connections is greater than 6 then it can't add another one
  if(room1.numberConnections >= 6)
  {
    result = 0;
  }
  return result;
}

//function that generates a random number between 0 and 6
int getRandomRoom()
{
  int randomNum = rand() % 7;
  return randomNum;
}

//function that checks if a connection between two rooms already exist
int connectionAlreadyExists(struct Room roomlist3[7], int room1, int room2)
{
  int result = 0;
  int i;
  for(i = 0; i < roomlist3[room2].numberConnections; i++)
  {
    if(roomlist3[room2].connections[i] == room1)
    {
      result = 1;
    }
  }
  return result;
}

//function that connects two rooms and updates number of connections
void connectRoom(struct Room rooms[7], int id1, int id2)
{
  rooms[id1].connections[rooms[id1].numberConnections] = id2;
  rooms[id2].connections[rooms[id2].numberConnections] = id1;
  rooms[id1].numberConnections = rooms[id1].numberConnections + 1;
  rooms[id2].numberConnections = rooms[id2].numberConnections + 1;
}

//function that checks if two rooms are the same
int isSameRoom(struct Room room1, struct Room room2)
{
  int result = 0;
  if(room1.name == room2.name)
  {
    result = 1;
  }
  return result;
}

//function that adds a random connection
void addRandomConnection(struct Room roomList3[7])
{
  int checker = 0;
  struct Room roomA;
  struct Room roomB;
  int tempHolder;
  int randRoom2;
  while(checker == 0)
  {
    int randRoom1 = getRandomRoom();
    roomA = roomList3[randRoom1];
    if(canAddConnectionFrom(roomA) == 1)
    {
      checker = 1;
      tempHolder = randRoom1;
    }
  }
  do
  {
    randRoom2 = getRandomRoom();
    roomB = roomList3[randRoom2];
  } while(canAddConnectionFrom(roomB) == 0 || isSameRoom(roomA, roomB) == 1 || connectionAlreadyExists(roomList3, tempHolder, randRoom2) == 1);
  connectRoom(roomList3, tempHolder, randRoom2);
}




void makeRooms(struct Room rooms[7])
{
  //room names
  int randRooms[10];
  int j;
  for(j = 0; j < 10; j++)
  {
    randRooms[j] = j;
  }
  int i;
  //shuffle the rooms
  for (i = 10 - 1; i > 0; --i)
  {
    // generate random index
    int w = rand() % i;
    // swap items
    int t = randRooms[i];
    randRooms[i] = randRooms[w];
    randRooms[w] = t;
  }
  i = 0;
  for(i = 0; i < 7; i++)
  {
    rooms[i].name = roomNames[randRooms[i]];
  }
  struct Room tempRoom;
  i = 0;
  rooms[0].roomType = types[0];
  //generate a random index for the end room
  int endRoomIndex = (rand() % (6 - 1 + 1)) + 1;
  rooms[endRoomIndex].roomType = types[2];
  i = 0;
  //all the rest of the rooms are mid rooms
  for(i = 0; i < 7; i++)
  {
    if(rooms[i].roomType != types[0] && rooms[i].roomType != types[2])
    {
      rooms[i].roomType = types[1];
    }
  }

  //room connections
  i = 0;
  for(i = 0; i < 7; i++)
  {
    rooms[i].numberConnections = 0;
    rooms[i].canAddConnection = 1;
  }
  //add connections until all rooms in list can't add another one
  while(isGraphFull(rooms) == 0)
  {
    addRandomConnection(rooms);
  }
  i = 0;
  for(i = 0; i < 7; i++)
  {
    j = 0;
    for(j = 0; j < rooms[i].numberConnections; j++)
    {
      int numConnect = j + 1;
    }
  }
}

//function that makes the room files and writes all appropriate data
void makeFiles(struct Room roomList[7], char* folder)
{
  FILE *output;
  //change the directory to the correct folder name
  chdir(folder);
  int j;
  int iter;
  for(iter = 0; iter < 7; iter++)
  {
    //create a file with room name
    output = fopen(roomList[iter].name, "a");
    if(output == NULL)
    {
      perror("Unable to create/open file.\n");
      return;
    }
    //write the room name in the file
    fprintf(output, "ROOM NAME: %s\n", roomList[iter].name);
    j = 0;
    for(j = 0; j < roomList[iter].numberConnections; j++)
    {
      //write the connections for that room
      int counter = j + 1;
      fprintf(output, "CONNECTION %d: %s\n", counter, roomList[roomList[iter].connections[j]].name);
    }
    //write the room type file
    fprintf(output, "ROOM TYPE: %s\n", roomList[iter].roomType);
    fclose(output);
  }
}

int main()
{
  //set a new seed
  srand(time(NULL));
  //create a list of rooms
  struct Room rooms1[7];
  char* folder1 = (char*)makeFolder();
  //make the rooms and create files
  makeRooms(rooms1);
  makeFiles(rooms1, folder1);
  return 0;
}
