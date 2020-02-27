/**********************************
*      Room Adventure Program     *
* Author: Bryce hahn			  *
* Email:    hahnb@oregonstate.edu *
**********************************/

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


char* getNewestDirectory() {
    char* search = "hahnb.rooms.";
    char* returnDir;
    int newestTime = 0;
    DIR* directory = opendir("."); //open local dir
    struct dirent *dir;
    struct stat st;
    size_t i;

    if (directory) { //make sure we actually opened
        while ((dir = readdir(directory)) != NULL) { //loop through all files and dirs
            if (strncmp(dir->d_name, search, 12) == 0) { //if the first 12 chars of
                    //the current dir matches search, we found a rooms dir.
                    returnDir = dir->d_name;
                    //check for most recent
                if (stat(returnDir, &st) != 0)
                    perror(returnDir);

                if (st.st_mtime > newestTime) {
                    newestTime = st.st_mtime;
                    printf("%s: -> %i", returnDir, ctime(newestTime));
                }
            }
        }
        closedir(directory);
    } else {
        perror("Invalid directory.");
    }
}

struct ROOM* readRooms(char* dir) {

}









int main(int argc, char** argv) {
    //get newest dir and load room details
    char* dir = getNewestDirectory();
    struct ROOM* rooms = readRooms(dir);

    //start game


    return 0;
}
