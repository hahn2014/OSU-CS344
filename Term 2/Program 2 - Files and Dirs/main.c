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
    int langCount;
    char** languages;
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
 *   https://www.geeksforgeeks.org/c-program-count-number-lines-file/
 */
int getLineCount(char* filename) {
    FILE* fp = fopen(filename, "r");
    char c;
    int count = 0;
    //loop through file looking for new line chars (get line count)
    for (c = getc(fp); c != EOF; c = getc(fp)) {
        if (c == '\n') // Increment count if this character is newline
            count++;
    }
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
    currMovie.year = (int)malloc(sizeof(int)); //allocate the proper memory size from new int
    currMovie.year = atoi(token); //now copy the data over

    //third token is the supported languages
    token = strtok_r(NULL, ",", &saveptr); //This should be:                    [a;b;c]

    //printf("%s\n", token); //for testing only (need to break up the languages into seperate strings)
    char* temp = (char*)malloc(255 * sizeof(char)); //allocate memory to copy strings to
    int i, j;
    for (i = 1; i < strlen(token) - 1; i++) { //start at 1 to cut out [ and end at -1 to cut out ]
        temp[i - 1] = token[i]; //make sure we start at index 0 on temp array
    }

    //printf("%s\n", temp); //at this point we have removed the brackets:       a;b;c

    //process each language by seperating each from the ;
    char** langs = (char**)malloc(20 * sizeof(char*));
    int count = 0;
    char* langToken = strtok(temp, ";");
    while (langToken != NULL) {
        // iterate through each token within the temp string and parse the tokens
        //printf("currentToken = %s\n", langToken);
        langs[count] = (char*)malloc(strlen(langToken) * sizeof(char));
        strcpy(langs[count], langToken);
        count++;
        langToken = strtok(NULL, ";"); // get next token
    }

    //force all language names to lower for string comparisons
    for (j = 0; j < count; j++) {
        for (i = 0; langs[j][i]; i++){
            langs[j][i] = tolower(langs[j][i]);
        }
    }

    currMovie.langCount = count;
    currMovie.languages = (char**)malloc(count * sizeof(char*)); //allocate enough space for (count) strings of languages

    for (i = 0; i < count; i++) {
        if (langs[i] != NULL) {
            //printf("%s\n", langs[i]);//finally we have each lang individually: a  \n  b  \n  c
            currMovie.languages[i] = (char*)malloc(strlen(langs[i]) * sizeof(char));
            currMovie.languages[i] = langs[i];
        }
    }

    //FREE USED MEMORY
    free(temp);
    free(langs);


    // //fourth token is the movie rating on a scale of 1 to 10
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie.rating = strtof(token, NULL);
    //currMovie->rating = (char*)malloc(strlen(token) * sizeof(char*));
    //strcpy(currMovie->rating, token);
    return currMovie;
}

struct movie* processFile(char* fileloc, int movieCount) {
    printf("reading file %s with %d entries\n", fileloc, movieCount);
    FILE* datafile = fopen(fileloc, "r");

    char* currLine = NULL; //initialize our line reading address
    size_t len = 0;
    ssize_t nread;
    char* token;
    int count = 0;

    struct movie* movies = (struct movie*)calloc(movieCount - 1, sizeof(struct movie)); //moviecount - 1 to excluse the file guide at the top

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
        printf("attempting to create new dir %s\n", dst);

        DIR* dir = opendir(dst);
        if (dir) { //directory exists
            printf("error! dir already exists.. running again.\n");
            closedir(dir);
            continue;
        } else if (ENOENT == errno) { //directory does not exist! Create it..
            if (mkdir(dst, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP) == 0) { //user can: R,W,E, group: R
                printf("%s succesfully created!\n", dst);

                count = getLineCount(filename);
                movies = processFile(filename, count); //add movie data to our struct array

                dir = opendir(dst);
                if (dir) {   //make sure we succesfully created and moved into the new dir
                    for (i = 1; i < count; i++) {
                        printf("Movie [%d/%d]: %d %s %.2f\n", i, (count - 1), movies[i].year, movies[i].title, movies[i].rating);

                        char* yearfile;
                        sprintf(yearfile, "%s/%d.txt", dst, movies[i].year);

                        FILE* year;
                        year = fopen(yearfile, "a+"); //a+ for appending a write to the end of the file
                        //since we are appending to already existing files, we can simply
                        //just keep writing till we looped through all movies in the list
                        fprintf(year, "%s\n", movies[i].title);
                        fclose(year);
                    }
                } else {
                    printf("Failed to create a directory.. Shutting down.\n");
                    exit(-1);
                }
            }
            exit(-1);
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
