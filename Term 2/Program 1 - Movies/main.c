#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int movie_count = 0;

struct movie {
    char* title;
    char* year;
    char* languages;
    char* rating;
    //for linked-list connectivity (only need a one way link)
    struct movie* next;
};

struct movie* addMovie(char* fileLine) {
    char* t, y, l, r;

    struct movie* currMovie = malloc(sizeof(struct movie)); //allocate memory for a new movie struct

    char* saveptr; //will be used while tokenizing the movie data from the char line

    //first token is the movie title
    char* token = strtok_r(fileLine, ",", &saveptr);
    currMovie->title = (char*)calloc(strlen(token) + 1, sizeof(char*)); //allocate the proper memory size from new token
    strcpy(currMovie->title, token); //now copy the data over

    //second token is the movie release year
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = (char*)calloc(strlen(token) + 1, sizeof(char*)); //allocate the proper memory size from new token
    strcpy(currMovie->year, token); //now copy the data over

    //third token is the supported languages
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->languages = (char*)calloc(strlen(token) + 1, sizeof(char*));
    strcpy(currMovie->languages, token);

    // //fourth token is the movie rating on a scale of 1 to 10
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie->rating = (char*)calloc(strlen(token) + 1, sizeof(char*)); //allocate the proper memory size from new token
    strcpy(currMovie->rating, token); //now copy the data over

    currMovie->next = NULL;
    return currMovie;
}



struct movie* processMovieFile(char* fileloc) {
    FILE* datafile = fopen(fileloc, "r");

    char* currLine = NULL; //initialize our line reading address
    size_t len = 0;
    ssize_t nread;
    char* token;

    //using a linked list of movie structs to hold our movie data
    struct movie* head = NULL;
    struct movie* tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, datafile)) != -1) {
        struct movie* newMovie = addMovie(currLine);

        if (head == NULL) { //first run only
            head = newMovie;
            tail = newMovie;
        } else { //anything after first run, add to last position
            tail->next = newMovie; //link previous tail to new tail
            tail = newMovie; //set tail to the new movie node
        }
        movie_count++;
    }

    free(currLine);
    fclose(datafile);
    return head;
}

void printMovie(struct movie* movie) {
    printf("%s, %s, %s, %s\n", movie->title, movie->year, movie->languages, movie->rating);
}

void printMovieList(struct movie* list) {
    while (list != NULL) {
        printMovie(list);
        list = list->next;
    }
}

void printByYear(struct movie* list) {
    int resp;
    printf("Enter the year for which you want to see movies: ");
    scanf("%d", &resp);
    while (list != NULL) {
        printf()
    }
}

int main(int argc, char** argv) {
    if (argc < 2) { //too few arguments, we need a movie file path
        printf("Error, you did not provide enough arguments! Syntax: ./MovieListing movie_sample_1.csv\n");
        return -1; //error break
    }
    struct movie* MovieList = processMovieFile(argv[1]);
    //printMovieList(MovieList); //for debugging only

    //now that the movie list is processed, give the user their 4 options
    printf("Processed file %s and parsed data for %i movies\n", argv[1], movie_count);
    int resp;

    while (1) {
        printf("1. Show movies released in the specified year\n2. Show highest rated movie for each year\n3. Show the title and year of release of all movies in a specific language\n4. Exit from the program\n\n");
        printf("Enter a choice from 1 to 4: ");
        //get user input
        scanf("%d", &resp);
        switch (resp) {
            case 1: //list movies from a specific year
                printByYear(MovieList);
            break;
            case 2: //show highest rated movie from each year

            break;
            case 3: //show the title and year of release of all movies in a specific language

            break;
            case 4: //exit the program
                printf("Thanks for using Bryce Hahn's Movie Listing program! Goodbye\n");
                return 0;
            break;
            default: //anything else and it should error out
                printf("You entered an incorrect choice. Try again.\n\n");
            break;
        };
    }
    return 0;
}
