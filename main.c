#include <stdio.h>
#include <stdlib.h> // calloc
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <string.h> // strstr
#include <ctype.h> // isdigit
#include <math.h> // numDigits()

#define STR_OBJECTS_HEADER "BEGIN Objects"
#define STR_TRACKERS_HEADER "BEGIN Trackers"
#define STR_HISTORICAL_TRACKERS_HEADER "BEGIN HistoricalTrackers"

#define VERSION "1.0"

#define ERR_NONE 0
#define ERR_NO_FILE_PATH 1
#define ERR_FILE_NOT_FOUND 2
#define ERR_FILE_EMPTY 3
#define ERR_READING_FILE 4
#define ERR_NO_OBJECTS_HEADING 5
#define ERR_NO_TRACKERS_HEADING 6
#define ERR_OUT_OF_PRISONERS 7
#define ERR_NO_ID 8
#define ERR_OTHER 99

const char* validItems[] = {
    "AssaultRifle",
    "Rifle"
};

typedef struct
{
    char *forname;
    int fornameLength;
    char *surname;
    int surnameLength;
    long IDi;
    long IDu;
} Prisoner;

size_t getFileSize(char* path);
//void editFile(char* buffer, size_t bufferSize, char* path);
int numDigits(int number);
void logError(FILE *fp, int errNum);
int getPrisonerIDs(
    char *buffer,
    char *bufferEnd,
    long *arr_IDi,
    long *arr_IDu,
    int *err
);
char *getNextToken(
    char *buffer,
    char *bufferEnd,
    int direction,
    int *tokenLength
);
char *getNextPrisoner(
    char *buffer,
    char *bufferEnd,
    int prisonerIndex,
    long *arr_IDi,
    long *arr_IDu,
    int *err
);
char *findTrackersHeadingEnd(
    char *buffer,
    char *bufferEnd,
    int *err
);

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
    char *buffer = calloc(size, 1);
    char *bufferEnd = buffer + size;

    // Reopen file to read into buffer
    FILE *fp = fopen(argv[1], "r");
    size_t bytesRead = fread(buffer, 1, size, fp);
    fclose(fp);

    // Make sure reading completed
    if (bytesRead < size)
    {
        logError(log_fp, 4);
        free(buffer);
        return 1;
    }

    int err = 0;

    fprintf(log_fp, "arm_prisoners %s\n", VERSION);

    // IDs
    long *arr_IDi = malloc(600 * sizeof(long));
    long *arr_IDu = malloc(600 * sizeof(long));
    fprintf(log_fp, "Acquiring prisoner IDs...\n");

    int numPrisoners = getPrisonerIDs(
            buffer, bufferEnd, arr_IDi, arr_IDu, &err);
    logError(log_fp, err);
    if (err != ERR_OUT_OF_PRISONERS) {
        free(buffer);
        return 1;
    }
    fprintf(log_fp, "Finished collecting %d prisoner IDs.\n", numPrisoners - 1);

    for (int i = 0; i < numPrisoners; i++) {
        fprintf(log_fp, "Prisoner %d: \t\tId.i %ld,\tId.u %ld\n", i, arr_IDi[i], arr_IDu[i]);
    }

    // Trackers
    fprintf(log_fp, "Removing existing trackers...\n");
    err = 0;
    char *trackers = findTrackersHeadingEnd(buffer, bufferEnd, &err);
    logError(log_fp, err);

    char *historicalTrackers = strstr(buffer, STR_HISTORICAL_TRACKERS_HEADER);
    assert(historicalTrackers);

    size_t trackersSize = historicalTrackers - trackers;
    fprintf(log_fp, "Initial size of trackers: %zd\n", trackersSize);

    if (!err)
        fprintf(log_fp, "All prisoners armed... God Bless America!!!\n");

    free(buffer);

    fclose(log_fp);

    return 0;
}

void logError(FILE *fp, int errNum) {
    switch(errNum) {
        case ERR_NONE:
            break;
        case ERR_NO_FILE_PATH:
            fprintf(fp, "ERROR: No file path provided.\n");
            break;
        case ERR_FILE_NOT_FOUND:
            fprintf(fp, "ERROR: File not found or file was empty.\n");
            break;
        case ERR_FILE_EMPTY:
            fprintf(fp, "ERROR: File not found or file was empty.\n");
            break;
        case ERR_READING_FILE:
            fprintf(fp, "ERROR: Could not finish reading file.\n");
            break;
        case ERR_NO_OBJECTS_HEADING:
            fprintf(fp, "ERROR: Program was unable to find Objects header in the file. Make sure this is a prison architect save file. If the problem persists, there may have been a change to the save file format and this program is out of data.\n");
            break;
        case ERR_NO_TRACKERS_HEADING:
            fprintf(fp, "ERROR: Program was unable to find Trackers header in the file. Make sure this is a prison architect save file. If the problem persists, there may have been a change to the save file format and this program is out of data.\n");
            break;
        case ERR_NO_ID:
            fprintf(fp, "ERROR: Unable to find prisoner ID.i or Id.u values. Save file format may have changed.\n");
            break;
        case ERR_OUT_OF_PRISONERS:
            fprintf(fp, "No more prisoners to read in.\n");
            break;
        default:
            fprintf(fp, "Error occurred: %d\n", errNum);
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

// Loads prisoner IDs into ID arrays and returns number of prisoners found
int getPrisonerIDs(char *buffer, char *bufferEnd, long *arr_IDi, long *arr_IDu, int *err) {
    // The part of the save file where objects and people are stored
    char *objectsAddr = strstr(buffer, STR_OBJECTS_HEADER);
    if (!objectsAddr) {
        *err = ERR_NO_OBJECTS_HEADING;
        return 0;
    }

    char *prisonerAddr = buffer;
    for (int i = 0;; i++) {
        prisonerAddr = getNextPrisoner(
                prisonerAddr, bufferEnd, i, arr_IDi, arr_IDu, err) + 1;
        if (*err) {
            return i;
        }
    }
}

// Returns the addr of the next grouping of characters for parsing
// Marks the first non-whitespace, then counts characters until next whitespace
char *getNextToken(char *buffer, char *bufferEnd, int direction, int *tokenLength) {
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
        if (&walker[i] == bufferEnd)
            return NULL;

        if (walker[i] > 32 && walker[i] < 127) {
            tokenStart = &walker[i];
            reachedTokenStart = true;
            walker = &walker[i];
        }
    }

    // Find length from start until next empty space
    for (i = 0; (walker[i] > 32 && walker[i] < 127); i += direction) {
        if (&walker[i] == bufferEnd)
            break;
    }

    *tokenLength = abs(i);

    // Move the start to the actual start of the word when direction is -1
    if (direction == -1) {
        tokenStart -= ((*tokenLength) - 1);
    }

    //printf("Length of word: %d\n", *tokenLength);

    return tokenStart;
}

// Finds next occurence of key and returns address of next token
char *getKeyValue(char *key, char *buffer, char *bufferEnd, int *valueLength) {
    int tokenLength = 0;
    char *token = buffer;

    // Find key
    do {
        token = getNextToken(token, bufferEnd, 1, &tokenLength);
    } while (strncmp(token, key, tokenLength) != 0);

    // Return token after key
    token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);

    *valueLength = tokenLength;
    return token;
}

char *getNextPrisoner(char *buffer, char *bufferEnd, int prisonerIndex, long *arr_IDi, long *arr_IDu, int *err) {
    // Find line where Type : Prisoner
    int tokenLength = 0;
    char *token = buffer;

    char *prisonerAddr = NULL;
    bool foundPrisoner = false;
    while (!foundPrisoner) {
        // Start searching for keyword "Type"
        while (strncmp(token, "Type", tokenLength) != 0) {
            token = getNextToken(
                    token + tokenLength, bufferEnd, 1, &tokenLength);

            if (token == NULL) {
                *err = ERR_OUT_OF_PRISONERS;
                return NULL;
            }
        }

        // Stop if object is of type prisoner
        token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);

        if (token == NULL) {
            *err = ERR_OUT_OF_PRISONERS;
            return NULL;
        } else if (strncmp(token, "Prisoner", tokenLength) == 0) {
            prisonerAddr = token;
            foundPrisoner = true;
        }
    }

    char *endptr;

    // Get Id.i
    while (strncmp(token, "Id.i", tokenLength) != 0) {
        token = getNextToken(token, bufferEnd, -1, &tokenLength);
    }
    token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);

    long IDi = strtol(token, &endptr, 10);
    if (endptr == token) {
        *err = ERR_NO_ID;
        return NULL;
    }

    // Get Id.u
    while (strncmp(token, "Id.u", tokenLength) != 0) {
        token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);
    }
    token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);

    long IDu = strtol(token, &endptr, 10);
    if (endptr == token) {
        *err = ERR_NO_ID;
        return NULL;
    }

    arr_IDi[prisonerIndex] = IDi;
    arr_IDu[prisonerIndex] = IDu;

    return prisonerAddr;
}

char *findTrackersHeadingEnd(char *buffer, char *bufferEnd, int *err) {
    char *token = buffer;
    int tokenLength = 0;
    token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);
    while (strncmp(token, "Trackers", tokenLength) != 0) {
        token = getNextToken(token + tokenLength, bufferEnd, 1, &tokenLength);
    }
    if (!token) {
        *err = ERR_NO_TRACKERS_HEADING;
        return NULL;
    }

    return token + tokenLength;

    /*char *trackersAddr = token;
    char *trackersEnd = NULL;
    trackersEnd = strstr(trackersAddr, "\nEND");
    assert(trackersEnd);
    for (int i = 0; i < 30; i++) {
        printf("%c", trackersEnd[i]);
    }
    printf("\n");*/
}

void addTracker(char *item, long IDi, long IDu) {
}
