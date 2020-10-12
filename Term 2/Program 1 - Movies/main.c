#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //for tolower
#include <string.h> //for strcpy, strcmp, strtok_r

int movie_count = 0;

struct movie {
    char* title;
    int year;
    int langCount;
    char** languages;
    char* rating;
    //for linked-list connectivity (only need a one way link)
    struct movie* next;
};

struct bestofyear {
    char* title;
    int year;
    float rating;
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
    currMovie->year = (int)malloc(sizeof(int)); //allocate the proper memory size from new int
    currMovie->year = atoi(token); //now copy the data over

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

    currMovie->langCount = count;
    currMovie->languages = (char**)malloc(count * sizeof(char*)); //allocate enough space for (count) strings of languages

    for (i = 0; i < count; i++) {
        if (langs[i] != NULL) {
            //printf("%s\n", langs[i]);//finally we have each lang individually: a  \n  b  \n  c
            currMovie->languages[i] = (char*)malloc(strlen(langs[i]) * sizeof(char));
            currMovie->languages[i] = langs[i];
        }
    }

    //FREE USED MEMORY
    free(temp);
    free(langs);


    // //fourth token is the movie rating on a scale of 1 to 10
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie->rating = (char*)malloc(strlen(token) * sizeof(char*));
    strcpy(currMovie->rating, token);
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
    printf("%s, %d, [", movie->title, movie->year);
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
        //printf("list year: %d, resp year: %d\n", list->year, resp);
        if (list->year == respyear) {
            printf("%s\n", list->title);
            count++;
        }
        list = list->next;
    }
    if (count == 0) {
        printf("No data about movies released in the year %d\n", respyear);
    }
}

void printBestofYear(struct movie* list, int n) {
    struct bestofyear BoY[n + 1];
    int count = 0, i, holdNewNode = 0;

    while (list != NULL) {
        for (i = 0; i < count; i++) { //check for existing year in BoY
            //printf("checking if %d == %d\n", BoY[i].year, list->year);
            if (BoY[i].year == list->year) {
                float ratingA = strtod(list->rating, NULL);
                float ratingB = BoY[i].rating;

                //printf("Checking if %f > %f\n", ratingA, ratingB);
                //check if list rating is better than BoY rating for cur year
                if (ratingA > ratingB) {
                    //printf("%f is the better rating... ", ratingA);
                    //make the new rating the BoY rating
                    BoY[i].title = list->title;
                    BoY[i].rating = ratingA;
                    holdNewNode = 1; //we are replacing the existing node
                    //printf("replacing existing [%d %f] node with: %d %f %s\n", BoY[i].year, ratingB, BoY[i].year, BoY[i].rating, BoY[i].title);
                    break; //go to next node in list
                } else {
                    holdNewNode = 1;
                    //printf("%f is the lower rating... skipping new node\n", ratingA);
                    break; // since we found a similar node, we dont want to add another node of the same years
                }
            }
        }
        //if we got here, we couldn't find an already existing movie from this year, free to add it at the end
        if (holdNewNode == 0) { //add a new node
            //add to end of best of year (count)
            BoY[count].title = list->title;
            BoY[count].year = list->year;
            BoY[count].rating = strtod(list->rating, NULL);
            //printf("New Movie Node [%d]: %d %f %s\n", count, BoY[count].year, BoY[count].rating, BoY[count].title);
            count++;
        } else {
            holdNewNode = 0;
        }
        list = list->next;
    }

    //sort in assending order
    //qsort(BoY, count, sizeof(BoY[0]), );

    //printf("\n\nBest of Year Rating List generated (%d):\n", count);
    for (i = 0; i < count; i++) {
        printf("%d %.2f %s\n", BoY[i].year, BoY[i].rating, BoY[i].title);
    }
}

void printByLanguage(struct movie* list) {
    char resplang[25];// = (char*)malloc(25 * sizeof(char));
    int count = 0, i;
    printf("Enter the language for which you want to see movies: ");
    scanf("%s", resplang);

    //force all input characters to lower for string comparisons
    for (i = 0; resplang[i]; i++){
        resplang[i] = tolower(resplang[i]);
    }

    while (list != NULL) {
        for (i = 0; i < list->langCount; i++) {
            if (strcmp(list->languages[i], resplang) == 0) { //if there is a matching language
                printf("%d %s\n", list->year, list->title);
                count++;
                break;
            }
        }
        list = list->next;
    }
    if (count == 0) {
        printf("No data about movies released in %s\n", resplang);
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
        printf("\n1. Show movies released in the specified year\n2. Show highest rated movie for each year\n3. Show the title and year of release of all movies in a specific language\n4. Exit from the program\n\n");
        printf("Enter a choice from 1 to 4: ");
        //get user input
        scanf("%d", &resp);
        switch (resp) {
            case 1: //list movies from a specific year
                printByYear(MovieList);
            break;
            case 2: //show highest rated movie from each year
                printBestofYear(MovieList, movie_count);
            break;
            case 3: //show the title and year of release of all movies in a specific language
                printByLanguage(MovieList);
            break;
            case 4: //exit the program
                printf("Thanks for using Bryce Hahn's Movie Listing program! Goodbye\n");
                return 0;
            break;
            default: //anything else and it should error out
                printf("You entered an incorrect choice. Try again.\n");
            break;
        };
    }
    return 0;
}
