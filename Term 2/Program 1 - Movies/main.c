#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int movie_count = 0;

struct movie {
    char* title;
    char* year;
    int langCount;
    char** languages;
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
    token = strtok_r(NULL, ",", &saveptr); //This should be:                    [a;b;c]

    //printf("%s\n", token); //for testing only (need to break up the languages into seperate strings)
    char* temp = (char*)malloc(255 * sizeof(char)); //allocate memory to copy strings to
    int i;
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

    currMovie->langCount = count;
    currMovie->languages = (char**)malloc(count * sizeof(char*)); //allocate enough space for (count) strings of languages

    for (i = 0; i < count; i++) {
        if (langs[i] != NULL) {
            //printf("%s\n", langs[i]);//finally we have each lang individually: a  \n  b  \n  c
            currMovie->languages[i] = (char*)malloc(strlen(langs[i]) * sizeof(char));
            currMovie->languages[i] = langs[i];
        }
    }


    // currMovie->languages = (char*)calloc(strlen(token) + 1, sizeof(char*));
    // strcpy(currMovie->languages, token);

    //FREE USED MEMORY
    free(temp);
    free(langs);


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
    struct movie* tmpHead = head->next;
    free(head); //free the first node as it's the CSV file guide
    free(currLine);
    fclose(datafile);
    return tmpHead;
}

void printMovie(struct movie* movie) {
    printf("%s, %s, [", movie->title, movie->year);
    int i;
    for (i = 0; i < movie->langCount; i++) {
        printf("%s", movie->languages[i]);
        if (i < movie->langCount - 1)
            printf(",");
    }
    printf("], %s\n", movie->rating);
}

void printMovieList(struct movie* list) {
    while (list != NULL) {
        printMovie(list);
        list = list->next;
    }
}

void printByYear(struct movie* list) {
    int respyear, listyear, count;
    printf("Enter the year for which you want to see movies: ");
    scanf("%d", &respyear);
    while (list != NULL) {
        //printf("list year: %s, resp year: %d\n", list->year, resp);
        listyear = atoi(list->year);
        if (listyear == respyear) {
            printf("%s\n", list->title);
            count++;
        }
        list = list->next;
    }
    if (count == 0) {
        printf("No data about movies released in the year %d\n\n", respyear);
    }
}

void printBestofYear(struct movie* list) {


    while (list != NULL) {


        list = list->next;
    }
}

void printByLanguage(struct movie* list) {
    char* resplang;
    int count, i;
    printf("Enter the language for which you want to see movies: ");
    scanf("%s", &resplang);
    while (list != NULL) {

        for (i = 0; i < list->langCount; i++) {
            if (list->languages[i] == resplang) {
                printf("%s %s\n", list->year, list->title);
                count++;
            }
        }
        list = list->next;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) { //too few arguments, we need a movie file path
        printf("Error, you did not provide enough arguments! Syntax: ./MovieListing movie_sample_1.csv\n");
        return -1; //error break
    }
    struct movie* MovieList = processMovieFile(argv[1]);
    printMovieList(MovieList); //for debugging only

    //now that the movie list is processed, give the user their 4 options
    printf("Processed file %s and parsed data for %i movies\n\n", argv[1], movie_count);
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
                printBestofYear(MovieList);
            break;
            case 3: //show the title and year of release of all movies in a specific language
                printByLanguage(MovieList);
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
