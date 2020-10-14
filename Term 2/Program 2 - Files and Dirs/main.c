#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //for tolower
#include <string.h> //for strcpy, strcmp, strtok_r, strrchr
#include <dirent.h> //directory stats
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



char* getSmallestFile() {
    DIR* curDir;
    struct dirent* dir;
    curDir = opendir("."); //open current directory
    char* ext;
    char* smallestFile = (char*)malloc(255 * sizeof(char));
    int size = 0, smallest = 0;
    FILE *fp;


    if (curDir) { //make sure curDir is not NULL
        while ((dir = readdir(curDir)) != NULL) {
            ext = strrchr(dir->d_name, '.'); //split string by the dot
            if (ext) {
                //printf("extension is: %s\n", ext + 1);
                if (strcmp(ext + 1, "csv") == 0) {
                    //printf("CSV extension found\n");
                    fp = fopen(dir->d_name, "r");
                    fseek(fp, 0L, SEEK_END); //seek to end of file (how many bytes long the file is)
                    size = ftell(fp); //get size
                    //printf("%s file size: %d bytes\n", dir->d_name, size);
                    if (size < smallest) { //check if new file is smaller than previous
                        //printf("%s is smaller than %s (%d < %d)\n", dir->d_name, smallestFile, size, smallest);
                        smallest = size;
                        smallestFile = dir->d_name;
                    } else {
                        if (smallest == 0) { //first run
                            smallest = size;
                            smallestFile = dir->d_name;
                        }
                    }
                }
            } //else, there was no extension to the file
        }
        closedir(curDir);
    }
    return smallestFile;
}

char* getLargestFile() {
    DIR* curDir;
    struct dirent* dir;
    curDir = opendir("."); //open current directory
    char* ext;
    char* largestFile = (char*)malloc(255 * sizeof(char));
    int size = 0, largest = 0;
    FILE *fp;


    if (curDir) { //make sure curDir is not NULL
        while ((dir = readdir(curDir)) != NULL) {
            ext = strrchr(dir->d_name, '.'); //split string by the dot
            if (ext) {
                //printf("extension is: %s\n", ext + 1);
                if (strcmp(ext + 1, "csv") == 0) {
                    //printf("CSV extension found\n");
                    fp = fopen(dir->d_name, "r");
                    fseek(fp, 0L, SEEK_END); //seek to end of file (how many bytes long the file is)
                    size = ftell(fp); //get size
                    //printf("%s file size: %d bytes\n", dir->d_name, size);
                    if (size > largest) { //check if new file is larger than previous
                        //printf("%s is larger than %s (%d > %d)\n", dir->d_name, largestFile, size, largest);
                        largest = size;
                        largestFile = dir->d_name;
                    } else {
                        if (largest == 0) { //first run
                            largest = size;
                            largestFile = dir->d_name;
                        }
                    }
                }
            } //else, there was no extension to the file
        }
        closedir(curDir); // no loose ends
    }
    return largestFile;
}

int checkForFile(char* filename) {
    FILE* file;
    char* ext;

    if ((file = fopen(filename,"r")) != NULL) {
        //file exists, now check for CSV file extension
        ext = strrchr(filename, '.'); //split string by the dot
        if (ext) { //filename includes extension
            if (strcmp(ext + 1, "csv") == 0) { //extension is a CSV format
                return 1;
            }
        }
        // file exists
        fclose(file); //no loose ends. clean up the memory leaks
    } //file doesn't exist
    return 0;
}


int processMovieFile(char* filename) {

    return 0;
}



int main(int argc, char** argv) {
    if (argc > 1) { //too many arguments, we only want a program name
        printf("Error, you provided too many arguments! Syntax: ./MovieDirs\n");
        return -1; //error break
    }
    int resp, i;
    char* fileToProcess = (char*)malloc(255 * sizeof(char));

    while (1) {
        printf("1.  Select file to process\n2. Exit from the program\n\n");
        printf("Enter a choice 1 or 2: ");
        //get user input
        scanf("%d", &resp);
        switch (resp) {
            case 1: //list movies from a specific year
                printf("\nWhich file do you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\n\n");
                printf("Enter a choice from 1 to 3: ");
                scanf("%d", &resp);
                switch (resp) {
                    case 1: //largest file
                        fileToProcess = getLargestFile();
                        printf("Now processing the chosen file named %s\n", fileToProcess);
                        processMovieFile(fileToProcess);
                        break;
                    case 2: //smallest file
                        fileToProcess = getSmallestFile();
                        printf("Now processing the chosen file named %s\n", fileToProcess);
                        processMovieFile(fileToProcess);
                        break;
                    case 3: //manually enter name
                        printf("Enter the complete file name: ");
                        scanf("%s", fileToProcess);

                        //force all input characters to lower for string comparisons
                        for (i = 0; fileToProcess[i]; i++){
                            fileToProcess[i] = tolower(fileToProcess[i]);
                        }

                        if (checkForFile(fileToProcess) == 1) {
                            printf("Now processing the chosen file named %s\n", fileToProcess);
                            processMovieFile(fileToProcess);
                        } else {
                            printf("The file %s was not found. Try again\n", fileToProcess);
                        }
                        break;
                    default: //invalid option
                        printf("Error! You entered an invalid response. Please choose 1-3\n\n");
                        break;
                };
            break;
            case 2: //exit the program
                printf("Thanks for using Bryce Hahn's Movie Dirs program! Goodbye\n");
                return 0;
            break;
            default: //anything else and it should error out
                printf("You entered an incorrect choice. Choose 1 or 2.\n");
            break;
        };
    }
    return 0;
}
