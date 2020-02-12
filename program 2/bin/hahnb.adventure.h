#ifndef ADVENTURE_H
#define ADVENTURE_H
//includes
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <String.h>
#include <dirent.h>

//global variables
struct ROOM {
    char* roomName;
    char* roomType;
    char** connections;
};



//prototypes
char* getNewestDirectory();
struct ROOM* readRooms(char* dir);

#endif
