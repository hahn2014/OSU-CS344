/**********************************
*      Generate Rooms Program     *
* Author: Bryce hahn			  *
* Email:    hahnb@oregonstate.edu *
**********************************/

//INCLUDES
#include "hahnb.buildrooms.h"

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
    int error = 0;//mkdir(directoryName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (!error)
        printf("Directory created\n");
    else {
        printf("Unable to create directory\n");
    }

    return directoryName;
}

char* roomNametoFileName(char* room) {
    char* filename;
    int fileiter = 0;
    srand(time(NULL)); //define seed of the pseudorandom time
    // remove all spaces, and add a _room to the end
    for (int i = 0; i < strlen(room); i++) {
		if (room[i] != ' ') { //ignore spaces
			filename[fileiter] = room[i];
            fileiter++;
		}
	}
    sprintf(filename, "%s%s", filename, "_room");
    printf("New room file name: %s\n", filename);
    return filename;
}

struct ROOM* declareRoomConnections(struct ROOM* room) {


    return NULL;
}

void generateRooms(char* directory) {
    struct ROOM* rooms = malloc(sizeof(struct ROOM) * 7);
    for (int i = 0; i < 7; i++) { //we need to generate 7 room files
        rooms[i].roomName = roomNames[1];
        rooms[i].fileName = roomNametoFileName(roomNames[1]);

        //Get Room Type
        if (i == 0) { //room_type is start
            rooms[i].roomType = "START_ROOM";
        } else if (i == 6) { //room_type is end
            rooms[i].roomType = "END_ROOM";
        } else { //room_type is mid
            rooms[i].roomType = "MID_ROOM";
        }
    }

    rooms = declareRoomConnections(rooms);

    //save to file
}

int main(int argc, char** argv) {
    //create process directory
    char* dir = initDirectory();
    generateRooms(dir);


    return 0;
}
