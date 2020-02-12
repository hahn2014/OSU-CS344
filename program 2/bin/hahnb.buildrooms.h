#ifndef BUILDROOMS_H
#define BUILDROOMS_H
//INCLUDES
#include <time.h> //random integer
#include <stdlib.h> //random integer
#include <unistd.h> //printf
#include <stdio.h> //getting process ID
#include <sys/stat.h> //making directories
#include <string.h> //for strcat
#include <stdio.h>  //writing to files
#include <time.h>

//global variables
char* roomNames[] = {
    "Grand Hall",
    "West Wing",
    "East Wing",
    "Basement",
    "Billiard",
    "Garden",
    "Kitchen",
    "Master Bedroom",
    "Garage",
    "Office",
};

struct ROOM {
    char* roomName;
    char* fileName;
    char* roomType;
    char** connections;
};

//prototypes
char* initDirectory();
char* intstrcat(char* a, int b);
char* roomNametoFileName(char* room);
void generateRooms(char* directory);
void generateRoomConnections(struct ROOM* rooms);
char* nextRandomName(struct ROOM* rooms);
void connectRoom(struct ROOM A, struct ROOM B);
int connectionAlreadyExists(struct ROOM A, struct ROOM B);

#endif
