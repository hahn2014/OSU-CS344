/**********************************
*      Generate Rooms Program     *
* Author: Bryce hahn			  *
* Email:    hahnb@oregonstate.edu *
**********************************/

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

char* intstrcat(char* a, int b) {
    static char str[100]; //static to allocate for returning
    sprintf(str, "%s%d", a, b); //concatenate the char* and int into one
    return str;
}

char* initDirectory() {
    char* directoryName = "hahnb.rooms.";
    directoryName = intstrcat(directoryName, getpid());
    //make the directory
    printf("New directory: %s\n", directoryName);
    int error = mkdir(directoryName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (!error)
        printf("Directory created\n");
    else {
        printf("Unable to create directory\n");
    }

    strcat(directoryName, "/");
    return directoryName;
}

char* roomNametoFileName(char* room) {
    char* filename = malloc(sizeof(char) * 8);
    int fileiter = 0, i;
    printf("DEBUG: %s\n", room);
    // remove all spaces, and add a _room to the end
    for (i = 0; i < strlen(room); i++) {
		if (room[i] != ' ') { //ignore spaces
			filename[fileiter] = room[i];
            fileiter++;
		}
	}
    printf("DEBUG: %s\n", filename);
    strcat(filename, "_room");
    //printf("New room file name: %s\n", filename);
    return filename;
}

void generateRoomConnections(struct ROOM* rooms) {
    struct ROOM* A;
    struct ROOM* B;
    int i;

    for (i = 0; i < sizeof(rooms); i++) {
        A = &rooms[i];
        do {
            B = &rooms[rand() % sizeof(rooms)];
        } while ((sizeof(B->connections) > 6) || (A->roomName == B->roomName) || connectionAlreadyExists(*A, *B) == 1);

        connectRoom(*A, *B);
    }
}

int connectionAlreadyExists(struct ROOM A, struct ROOM B) {
    int i;
    for (i = 0; i < sizeof(A.connections); i++) {
        if (A.connections[i] == B.roomName) {
            return 1;
        }
    }
    return 0;
}

void connectRoom(struct ROOM A, struct ROOM B) {
    A.connections[sizeof(A.connections) + 1] = B.roomName;
    B.connections[sizeof(B.connections) + 1] = A.roomName;
}

char* nextRandomName(struct ROOM* rooms) {
    char** leftovers = malloc(sizeof(char*) * sizeof(roomNames));
    int iter, isTaken, i, j = 0;

    for (i = 0; i < sizeof(roomNames); i++) {
        isTaken = 0;
        for (j = 0; j < sizeof(rooms); j++) {
            if (rooms[j].roomName == roomNames[i]) {
                isTaken = 1;
                break; //already taken, go to next name
            }
        } //we got out of the second for loop, name is available
        if (isTaken == 0) {
            leftovers[iter] = roomNames[i];
            iter++;
        }
    }
    srand (time(NULL));
    printf("TEST new name");
    for (i = 0; i < sizeof(leftovers); i++) {
        printf("leftovers[%d] = %s", i, leftovers[i]);
    }


    //pick random name
    return leftovers[(rand() % sizeof(leftovers))];
}

void generateRooms(char* directory) {
    struct ROOM* rooms = malloc(sizeof(struct ROOM) * 7);
    FILE *fp; //so we can save to file on the fly
    char* file = directory;
    int i, j;

    for (i = 0; i < 7; i++) { //we need to generate 7 room files
        rooms[i].roomName = nextRandomName(rooms);
        rooms[i].fileName = roomNametoFileName(rooms[i].roomName);

        //Get Room Type
        if (i == 0) { //room_type is start
            rooms[i].roomType = "START_ROOM";
        } else if (i == 6) { //room_type is end
            rooms[i].roomType = "END_ROOM";
        } else { //room_type is mid
            rooms[i].roomType = "MID_ROOM";
        }
    }
    //generate room connections and type
    generateRoomConnections(rooms);

    for (i = 0; i < 7; i++) {
        file = directory;
        strcat(file, rooms[i].fileName);
        printf("attempting to open: %s\n", file);
        fp = fopen(file, "w+");

        //test if file opened
        if (!fp) {
            printf("Error! File was unable to be opened: %s", file);
            return;
        }


        fprintf(fp, "ROOM NAME: %s\n", rooms[i].roomName);

        for (j = 0; j < sizeof(rooms[i].connections); j++) {
            fprintf(fp, "CONNECTION %d: %s\n", j, rooms[i].connections[j]);
        }
        //sprintf(writeChar, "%s%s", writeChar, rooms[i].roomType);
        fprintf(fp, "ROOM TYPE: %s", rooms[i].roomType);
        fclose(fp);
    }

    //cleanup
    free(rooms);
}

int main(int argc, char** argv) {
    //create process directory
    char* dir = initDirectory();
    generateRooms(dir);
    return 0;
}
