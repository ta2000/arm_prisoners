#include <stdio.h>
#include <stdlib.h> // calloc
#include <string.h> // strstr
#include <ctype.h> // isdigit
#include <math.h> // numDigits()

size_t getFileSize(char* path);
void editFile(char* buffer, size_t bufferSize, char* path);
int numDigits(int number);

int main(int argc, char* argv[])
{
    // Check arguments
    if (argc != 2)
    {
        fprintf(stderr, "No file path provided, or too many arguments.\n");
        return 1;
    }

    // Check file size
    size_t size = getFileSize(argv[1]);
    if (size == 0)
    {
        fprintf(stderr, "File size is 0 bytes.\n");
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
        fprintf(stderr, "Did not finish reading file.\n");
        return 1;
    }

    editFile(buffer, size, argv[1]);
    free(buffer);

    return 0;
}

int numDigits(int number)
{
    if (number == 0)
        return 1;
    else
        return floor(log10(abs(number)))+1;
}

size_t getFileSize(char* path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File not found: %s\n", path);
        return 1;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    fclose(fp);

    return size;
}

void editFile(char* buffer, size_t bufferSize,char* path)
{
    // ====================== //
    // GET PRISONER ID.I/ID.U //
    // ====================== //


    // 2D array containing Id.i and Id.u of each prisoner
    int index = 0;
    long IDs[1000][2] = {0}; // Max 1000 prisoners

    // Find first prisoner in file
    char* currentPrisoner = strstr(buffer, "Type                 Prisoner  ");
    char* nextPrisoner = strstr(currentPrisoner+1, "Type                 Prisoner  ");

    while (currentPrisoner != NULL)
    {
        // Go back 2 lines to get Id.i and Id.u
        char* i; int lines = 0;
        for (i=(currentPrisoner-1); lines < 3; i--)
        {
            if (*i == '\n')
                lines++;
        }

        // For strtol
        char* endptr;

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


    // ======================== //
    // REMOVE EXISTING TRACKERS //
    // ======================== //

    // Find Trackers section
    char* i = strstr(buffer, "BEGIN Trackers");
    // Skip past size line
    int lines = 0;
    for (i; lines < 2; i++)
    {
        if (*i == '\n')
            lines++;
    }
    char* trackerStart = i;

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
    char* newBuffer = calloc(bufferSize + newChars, 1);
    // Copy information from old buffer to new buffer
    memcpy(newBuffer, buffer, bufferSize);

    char* iNew = strstr(newBuffer, "BEGIN HistoricalTrackers");

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
