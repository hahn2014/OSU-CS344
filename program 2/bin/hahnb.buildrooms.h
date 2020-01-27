#ifndef BUILDROOMS_H
#define BUILDROOMS_H
//INCLUDES
#include <time.h> //random integer
#include <stdlib.h> //random integer
#include <unistd.h> //printf
#include <stdio.h> //getting process ID
#include <sys/stat.h> //making directories
#include <string.h> //for strcat

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
    char roomName[8];
    char fileName[32];
    char roomType[10];
    char* connections[6];
};

//prototypes
char* initDirectory();
char* intstrcat(char* a, int b);
char* roomNametoFileName(char* room);
void generateRooms(char* directory);
struct ROOM* declareRoomConnections(struct ROOM* room);

#endif
