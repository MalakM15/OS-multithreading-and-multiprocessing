////Malak Milhem   1220031
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>

#define WORDLENGTH 50
#define NUMOFWORDS 300000

typedef struct {
    char word[WORDLENGTH];
    int frequency;
} WordFreq;

// Function to find a word in the word list
int findWord(WordFreq *wordList, int wordCount, const char *word) {
    for (int i = 0; i < wordCount; i++) {
        if (strcmp(wordList[i].word, word) == 0) {
            return i; // Found the word, return its index
        }
    }
    return -1; // Word not found
}

// Function to insert a word into the word list
void insertWord(WordFreq *wordList, int *wordCount, const char *word, int frequency) {
    int index = findWord(wordList, *wordCount, word);
    if (index != -1) {
        wordList[index].frequency += frequency;
    } else {
        strcpy(wordList[*wordCount].word, word);
        wordList[*wordCount].frequency = frequency;
        (*wordCount)++;
    }
}

// Process a chunk of text and update the local word list
void processPart(const char *text, size_t textSize, WordFreq *localWordList, int *localWordCount) {
    char word[WORDLENGTH];
    size_t index = 0;

    while (index < textSize) {
        int wordLen = 0;

        // Extract a word
        while (index < textSize && !isspace(text[index])) {
            if (wordLen < WORDLENGTH - 1) {
                word[wordLen++] = text[index];
            }
            index++;
        }
        word[wordLen] = '\0';

        if (wordLen > 0) {
            insertWord(localWordList, localWordCount, word, 1);
        }
        while (index < textSize && isspace(text[index])) {
            index++;
        }
    }
}

// Compare function for sorting word frequencies
int compareWordFreq(const void *a, const void *b) {
    return ((WordFreq *)b)->frequency - ((WordFreq *)a)->frequency;
}

int main() {
    FILE *file = fopen("/mnt/c/Users/user/Downloads/text8.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    struct timeval start, end;
    double timetaken;
    int numProcesses;
    printf("Enter number of processes: ");
    scanf("%d", &numProcesses);

    // Determine file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate memory for file content
    char *fileContent = malloc(fileSize + 1);
    if (!fileContent) {
        perror("Failed to allocate memory for file content");
        fclose(file);
        return 1;
    }
    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0'; // Null-terminate the content
    fclose(file);

    size_t textSize = fileSize / numProcesses;
    pid_t pids[numProcesses];
    gettimeofday(&start, NULL);

    // Create child processes and divide the work
    for (int i = 0; i < numProcesses; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Child process
            size_t startindex = i * textSize;
            size_t endindex = (i == numProcesses - 1) ? fileSize : startindex + textSize;

            // Adjust chunk boundaries to avoid splitting words
            while (startindex > 0 && !isspace(fileContent[startindex - 1])) {
                startindex--;
            }
            while (endindex < fileSize && !isspace(fileContent[endindex])) {
                endindex++;
            }

            WordFreq *localWordList = malloc(sizeof(WordFreq) * NUMOFWORDS);
            int localWordCount = 0;
            processPart(fileContent + startindex, endindex - startindex, localWordList, &localWordCount);

            // Write local word list to a file
            char filename[50];
            sprintf(filename, "process_%d.txt", i);
            FILE *outputFile = fopen(filename, "w");
            if (!outputFile) {
                perror("Failed to create output file");
                free(localWordList);
                exit(1);
            }

            for (int j = 0; j < localWordCount; j++) {
                fprintf(outputFile, "%s %d\n", localWordList[j].word, localWordList[j].frequency);
            }
            fclose(outputFile);
            free(localWordList);
            exit(0);
        }
    }

    // Parent process waits for all child processes
    for (int i = 0; i < numProcesses; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Merge results from child files
    WordFreq *globalWordList = malloc(sizeof(WordFreq) * NUMOFWORDS);
    int globalWordCount = 0;

    for (int i = 0; i < numProcesses; i++) {
        char filename[50];
        sprintf(filename, "process_%d.txt", i);
        FILE *inputFile = fopen(filename, "r");
        if (!inputFile) {
            perror("Failed to open child output file");
            continue;
        }

        char word[WORDLENGTH];
        int frequency;
        while (fscanf(inputFile, "%s %d", word, &frequency) != EOF) {
            insertWord(globalWordList, &globalWordCount, word, frequency);
        }
        fclose(inputFile);
        remove(filename); // Optionally remove the file after processing
    }

    // Sort the global word list
    qsort(globalWordList, globalWordCount, sizeof(WordFreq), compareWordFreq);

    gettimeofday(&end, NULL);
    timetaken = (end.tv_sec - start.tv_sec) * 1000000.0;
    timetaken += (end.tv_usec - start.tv_usec);

    // Print top 10 words
    printf("Top 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < globalWordCount; i++) {
        printf("%s: %d\n", globalWordList[i].word, globalWordList[i].frequency);
    }

    printf("Time taken: %.5f seconds\n", timetaken / 1000000.0);

    free(fileContent);
    free(globalWordList);
    return 0;
}
