#include <stdio.h>
#include <stdlib.h> // calloc
#include <stdbool.h>
#include <unistd.h>
#include <string.h> // strstr
#include <ctype.h> // isdigit
#include <math.h> // numDigits()

#define STR_OBJECTS_HEADER "BEGIN Objects"

#define VERSION 1.0

#define ERR_NONE 0
#define ERR_NO_FILE_PATH 1
#define ERR_FILE_NOT_FOUND 2
#define ERR_FILE_EMPTY 3
#define ERR_READING_FILE 4
#define ERR_NO_OBJECTS 5
#define ERR_OTHER 99

size_t getFileSize(char* path);
//void editFile(char* buffer, size_t bufferSize, char* path);
int numDigits(int number);
void logError(FILE *fp, int errNum);
bool isAlphaNumeric(char c);
int getNextPrisoner(char *buffer, long **arr_IDi, long **arr_IDu);

char* logFileName = "log.txt";

int main(int argc, char *argv[])
{
    FILE *log_fp = fopen(logFileName, "w+");

    // Check arguments
    if (argc < 2)
    {
        logError(log_fp, ERR_NO_FILE_PATH);
        return 1;
    }

    // Check file size
    size_t size = getFileSize(argv[1]);
    if (size == 0)
    {
        logError(log_fp, ERR_FILE_NOT_FOUND);
        return 1;
    }

    // Buffer to store file contents
    char* buffer = calloc(size, 1);

    // Reopen file to read into buffer
    FILE *fp = fopen(argv[1], "r");
    size_t bytesRead = fread(buffer, 1, size, fp);
    fclose(fp);

    // Make sure reading completed
    if (bytesRead < size)
    {
        return 1;
    }

    long *arr_IDi = malloc(600 * sizeof(long));
    long *arr_IDu = malloc(600 * sizeof(long));
    getNextPrisoner(buffer, &arr_IDi, &arr_IDu);
    //editFile(buffer, size, argv[1]);

    free(buffer);

    fclose(log_fp);

    return 0;
}

void logError(FILE *fp, int errNum) {
    switch(errNum) {
        case ERR_NO_FILE_PATH:
            fprintf(fp, "No file path provided.\n");
            break;
        case ERR_FILE_NOT_FOUND:
            fprintf(fp, "File not found or file was empty.\n");
            break;
        case ERR_FILE_EMPTY:
            fprintf(fp, "File not found or file was empty.\n");
            break;
        case ERR_READING_FILE:
            fprintf(fp, "Could not finish reading file.\n");
            break;
        case ERR_NO_OBJECTS:
            fprintf(fp, "Program was unable to find Objects header in the file. Make sure this is a prison architect save file. If the problem persists, there may have been a change to the save file format and this program is out of data.\n");
            break;
        default:
            fprintf(fp, "Error occurred\n");
            break;
    }
}

int numDigits(int number) {
    if (number == 0)
        return 1;
    else
        return floor(log10(abs(number)))+1;
}

size_t getFileSize(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File not found: %s\n", path);
        return 0;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    fclose(fp);

    return size;
}

/*
void editFile(char *buffer, size_t bufferSize, char *path) {
    // ======================== //
    // REMOVE EXISTING TRACKERS //
    // ======================== //

    // Find Trackers section
    char *i = strstr(buffer, "BEGIN Trackers");
    // Skip past size line
    int lines = 0;
    for (i; lines < 2; i++)
    {
        if (*i == '\n')
            lines++;
    }
    char *trackerStart = i;

    // Navigate to the section after Trackers
    i = strstr(trackerStart, "BEGIN HistoricalTrackers");
    // Move up 1 line to get the end of Trackers
    lines = 0;
    for (i; lines < 2; i--)
    {
        if (*i == '\n')
            lines++;
    } i++;
    printf("Number of chars to be removed: %d\n", (i-trackerStart));
    memmove(trackerStart, i, (i-trackerStart));
    memset(i, 0, (i-trackerStart));


    // ================ //
    // ADD NEW TRACKERS //
    // ================ //

    // Find number of new chars needed
    unsigned long newChars = 1112;

    // Amount of chars for assault rifle tracker, multiplied by num prisoners.
    newChars += (200 * index) + 1;

    // Amount of chars from numbers that change:
    // ID of tracker - BEGIN "[i X]"
    // Id.i/Id.u of the prisoner
    int j;
    for (j=0; j<index; j++)
    {
        newChars += numDigits(j); // Tracker number
        newChars += numDigits(IDs[index][0]); // Id.i
        newChars += numDigits(IDs[index][1]); // Id.u
    }
    printf("New chars to be added: %lu\n", newChars);

    // Create a new buffer, large enough to include new Trackers
    char *newBuffer = calloc(bufferSize + newChars, 1);
    // Copy information from old buffer to new buffer
    memcpy(newBuffer, buffer, bufferSize);

    char *iNew = strstr(newBuffer, "BEGIN HistoricalTrackers");

    // Make space for new trackers to be written
    memmove(iNew+newChars, iNew, newChars);
    memset(iNew, 0, newChars);

    iNew = strstr(newBuffer, "BEGIN Trackers");
    // Move down 2 line to the middle of Trackers
    lines = 0;
    for (iNew; lines < 2; iNew++)
    {
        if (*iNew == '\n')
            lines++;
    }

    // Add newline at beginning
    memcpy(iNew, "\n", 1); iNew++;

    // Give each prisoner an assault rifle
    for (j=0; j<index; j++)
    {
        // BEGIN "[i j]"
        memcpy(iNew, "\t\tBEGIN \"[i ", 12);
        iNew += 12;
        sprintf(iNew, "%d", j);
        iNew += numDigits(j);
        memcpy(iNew, "]\"\t \n", 5);
        iNew += 5;

        // ItemType             AssaultRifle
        memcpy(iNew, "\t\t\tItemType\t\t\t AssaultRifle  \n", 30);
        iNew += 30;

        // State                Owned
        memcpy(iNew, "\t\t\tState\t\t\t\t Owned  \n", 21);
        iNew += 21;

        // BirthTime            18074.42
        memcpy(iNew, "\t\t\tBirthTime\t\t\t 18074.42  \n", 27);
        iNew += 27;

        // Prisoner.i
        memcpy(iNew, "\t\t\tPrisoner.i  \t\t ", 18);
        iNew += 18;
        sprintf(iNew, "%d", IDs[j][0]);
        iNew += numDigits(IDs[j][0]);
        memcpy(iNew, "  \n", 3);
        iNew += 3;

        // Prisoner.u
        memcpy(iNew, "\t\t\tPrisoner.u  \t\t ", 18);
        iNew += 18;
        sprintf(iNew, "%d", IDs[j][1]);
        iNew += numDigits(IDs[j][1]);
        memcpy(iNew, "  \n", 3);
        iNew += 3;

        // Prisoner.chance      1.000000
        memcpy(iNew, "\t\t\tPrisoner.chance \t 1.000000  \n", 32);
        iNew += 32;

        // Log
        memcpy(iNew, "\t\t\tBEGIN Log   \t \n", 18);
        iNew += 18;
        memcpy(iNew, "\t\t\tEND\n", 7);
        iNew += 7;

        // END
        memcpy(iNew, "\t\tEND\n", 6);
        iNew += 6;
    }

    // Overwrite file
    FILE *fp = fopen(path, "w+");
    size_t bytesWritten = fwrite(newBuffer, 1, bufferSize + newChars, fp);
    fclose(fp);

    free(newBuffer);
}
*/

bool isAlphaNumeric(char c) {
    return ((c >= 48 && c <= 57) ||
            (c >= 65 && c <= 90) ||
            (c >= 97 && c <= 122));
}

// Returns the addr of the next grouping of characters for parsing
// Marks the first non-whitespace, then counts characters until next whitespace
char *getNextToken(char *buffer, int direction, int *tokenLength) {
    char *tokenStart;
    char *walker = buffer;
    bool reachedTokenStart = false;

    // Direction must be -1 or 1
    if (direction * direction != 1) {
        printf("Direction in getNextToken not -1 or 1\n");
        return NULL;
    }

    // Find where next token starts
    // If searching in reverse, start 1 before
    int i = (direction == -1) ? -1 : 0;
    for(i; !reachedTokenStart; i += direction) {
        if (walker[i] > 32 && walker[i] < 127) {
            tokenStart = &walker[i];
            reachedTokenStart = true;
            walker = &walker[i];
        }
    }

    // Find length of token
    for (i = 0; (walker[i] > 32 && walker[i] < 127); i += direction);
    printf("\n");
    *tokenLength = abs(i);

    // Move the start to the actual start of the word when direction is -1
    if (direction == -1) {
        tokenStart -= ((*tokenLength) - 1);
    }

    printf("Length of word: %d\n", *tokenLength);

    return tokenStart;
}

int getNextPrisoner(char *buffer, long **arr_IDi, long **arr_IDu) {
    int index = 0;
    //long IDs[1000][2] = {0}; // Max 1000 prisoners

    // The part of the save file where objects and people are stored
    char *objects_addr = strstr(buffer, STR_OBJECTS_HEADER);
    if (!objects_addr) {
        return ERR_NO_OBJECTS;
    }

    for (int i = 0; i < 200; i++)
        printf("%c", objects_addr[i]);
    printf("\n\n");

    /*char *currentPrisoner = NULL;
    bool foundPrisoner = false;
    while (!foundPrisoner) {
        // Look for type prisoner on this line
        strstr(objects_addr, "Type");
        for (int i = 0; objects_addr[i] != '\n'; i++) {
            if (strncmp("Prisoner", objects_addr + i, sizeof("Prisoner") - 1)) {
                foundPrisoner = true;
                break;
            }
        }
    }*/

    int tokenLength = 0;
    //char *tokenStart = getNextToken(objects_addr, 1, &tokenLength);
    char *tokenStart = objects_addr;
    for (int i = 0; i < 10; i++) {
        tokenStart = getNextToken(tokenStart, -1, &tokenLength);
        for (int j = 0; j < tokenLength; j++)
            printf("%c", tokenStart[j]);
        printf("\n");
        //sleep(1);
    }

    return 0;

    // Find first prisoner in file
    //char *currentPrisoner = strstr(objects_addr, "Type                 Prisoner  ");
    //char *nextPrisoner = strstr(currentPrisoner+1, "Type                 Prisoner  ");

    /*
    while (currentPrisoner != NULL)
    {
        // Go back 2 lines to get Id.i and Id.u
        char *i; int lines = 0;
        for (i=(currentPrisoner-1); lines < 3; i--)
        {
            if (*i == '\n')
                lines++;
        }

        // For strtol
        char *endptr;

        // Move forward until the Id.i number is found
        for (i; !isdigit(*i); i++);
        // Convert Id.i string to long and add to array
        IDs[index][0] = strtol(i, &endptr, 10);

        // Move to end of number
        i = endptr;

        // Move forward to next line until Id.u is found
        for (i; !isdigit(*i); i++);
        // Convert Id.i string to long and add to array
        IDs[index][1] = strtol(i, &endptr, 10);

        // Print IDs of prisoner
        //printf("Id.i: %d\nId.u: %d\n\n", IDs[index][0], IDs[index][1]);

        // Make the next prisoner current
        currentPrisoner = nextPrisoner;
        // If this is not the last prisoner, look for the next one
        if (currentPrisoner != NULL)
        {
            nextPrisoner = strstr(currentPrisoner+1, "Type                 Prisoner  ");
        }

        // Increment index for next prisoner
        index++;
    }
*/

}

void removeTrackers() {
}

void addTracker() {
}
