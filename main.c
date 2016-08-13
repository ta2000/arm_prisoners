#include <stdio.h>
#include <stdlib.h> // calloc
#include <string.h> // strstr
#include <ctype.h> // isdigit

size_t getFileSize(char* path);
void editFile(char* buffer);

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
        fprintf(stderr, "File size is 0 bytes.");
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

    editFile(buffer);
    free(buffer);

    return 0;
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

void editFile(char* buffer)
{
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

        printf("Id.i: %d\nId.u: %d\n\n", IDs[index][0], IDs[index][1]);

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
}
