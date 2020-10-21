#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //for tolower
#include <string.h> //for strcpy, strcmp, strtok_r, strrchr
#include <dirent.h> //directory stats
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <time.h>  //for srand seeding
#include <unistd.h>

struct movie {
    char* title;
    int year;
    float rating;
};

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

/*
 * ITOA function courtesy of
 *   http://www.strudel.org.uk/itoa/
 */
char* itoa(int val) {

	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= 10)

		buf[i] = "0123456789abcdef"[val % 10];

	return &buf[i+1];
}

/*
 * get line count function courtesy of
 *   https://www.tutorialspoint.com/c-program-to-count-the-number-of-lines-in-a-file
 */
int getLineCount(char* filename) {
    FILE* fp = fopen(filename, "r");
    char ch;
    int count = 0;
    //open file in read more
    //printf("getting line count of %s\n", filename);
    if (fp == NULL) {
        printf("File \"%s\" does not exist!!!\n", filename);
        return -1;
    }

    //read character by character and check for new line
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    //close the file
    fclose(fp);
    return count;
}


struct movie addMovie(char* fileLine) {
    char* t, y, l, r;

    struct movie currMovie; //allocate memory for a new movie struct

    char* saveptr; //will be used while tokenizing the movie data from the char line

    //first token is the movie title
    char* token = strtok_r(fileLine, ",", &saveptr);
    currMovie.title = (char*)calloc(strlen(token) + 1, sizeof(char*)); //allocate the proper memory size from new token
    strcpy(currMovie.title, token); //now copy the data over

    //second token is the movie release year
    token = strtok_r(NULL, ",", &saveptr);
    //currMovie.year = (int)malloc(sizeof(int)); //allocate the proper memory size from new int
    currMovie.year = atoi(token); //now copy the data over

    //third token is the supported languages
    token = strtok_r(NULL, ",", &saveptr); //This should be:                    [a;b;c]

        //we can skip this part because we don't need languages

    // //fourth token is the movie rating on a scale of 1 to 10
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie.rating = strtof(token, NULL);
    return currMovie;
}

struct movie* processFile(char* fileloc, int movieCount) {
    //printf("reading file %s with %d entries\n", fileloc, movieCount);
    FILE* datafile = fopen(fileloc, "r");

    char* currLine = NULL; //initialize our line reading address
    size_t len = 0;
    ssize_t nread;
    char* token;
    int count = 0;

    struct movie* movies = (struct movie*)calloc(movieCount, sizeof(struct movie)); //moviecount - 1 to excluse the file guide at the top

    // Read the file line by line
    while ((nread = getline(&currLine, &len, datafile)) != -1) {
        if (count > 0) {
            movies[count] = addMovie(currLine);
            //printf("debug: New Movie Added [%s, %d, %.2f]\n", movies[count].title, movies[count].year, movies[count].rating);
        } //skip the first entry, as it will ALWAYS be a CSV file template guide
        count++; //increment lines read
    }

    free(currLine);
    fclose(datafile);
    return movies;
}

void processMovieFile(char* filename) {
    int randIndex, i, j, count;
    struct movie* movies;
    while (1) { //keep generating random names till a free directory is found
        randIndex = rand() % 100000;
        char dst[20] = "hahnb.movies.";
        strcat(dst, itoa(randIndex));
        //printf("attempting to create new dir %s\n", dst);

        DIR* dir = opendir(dst);
        if (dir) { //directory exists
            //printf("error! dir already exists.. running again.\n");
            closedir(dir);
            continue;
        } else if (ENOENT == errno) { //directory does not exist! Create it..
            if (mkdir(dst, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP) == 0) { //user can: R,W,E, group: R
                //printf("%s succesfully created!\n", dst);

                count = getLineCount(filename);
                movies = processFile(filename, count); //add movie data to our struct array

                dir = opendir(dst);
                if (dir) {   //make sure we succesfully created and moved into the new dir
                    printf("Created directory with name %s\n\n", dst);
                    for (i = 1; i < count; i++) {
                        char* yearfile = (char*)malloc(8 * sizeof(char)); //year (4) + . (1) + txt (3) = 8
                        sprintf(yearfile, "%s/%d.txt", dst, movies[i].year);
                        //printf("new year file: %s\n", yearfile);

                        FILE* year = fopen(yearfile, "a+"); //a+ for appending a write to the end of the file
                        //     //since we are appending to already existing files, we can simply
                        //     //just keep writing till we looped through all movies in the list
                        fprintf(year, "%s\n", movies[i].title);
                        fclose(year);
                    }
                    closedir(dir);
                    return; //at this point we've gone through all the movies, go back to main menu
                } else {
                    printf("Failed to create a directory.. Try again.\n");
                    return;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    //initialize random
    srand(time(0));

    //check for args
    if (argc > 1) { //too many arguments, we only want a program name
        printf("Error, you provided too many arguments! Syntax: ./MovieDirs\n");
        return -1; //error break
    }
    int resp, i;
    char* fileToProcess = (char*)malloc(255 * sizeof(char));

    while (1) {
        printf("1. Select file to process\n2. Exit from the program\n\n");
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
