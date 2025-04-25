//Naïve approach with array implementation
//Malak Milhem   1220031

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define WORDLENGTH 50
#define NUMOFWORDS 300000

typedef struct{
    char word[WORDLENGTH];
    int frequency;
}WordFreq;

int wordcount = 0;  //num of unique words
 WordFreq wordList [NUMOFWORDS];
int findWord(const char *word){
    for (int i =0; i<wordcount ; i++){
        if (strcmp(wordList[i].word, word) == 0){
            return i; //the word's index
        }
    }
    return -1 ; //word not found
}

void insertWord (const char *word){
    int index = findWord (word);
    if (index != -1){
    wordList[index].frequency++;
    } else {
        strcpy(wordList[wordcount].word,word);
         wordList[wordcount].frequency = 1;
        wordcount++;
    }
}

int main(){

 FILE *file = fopen("/mnt/c/Users/user/Downloads/text8.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

   
char word [WORDLENGTH];
clock_t startTime = clock(); //

while (fscanf(file, "%49s", word) != EOF) {
            insertWord(word);
        }
   
 fclose(file); 

 for (int i = 0; i < (wordcount - 1); i++) {   // sort final list of words
        for (int j = i + 1; j < wordcount; j++) {
            if (wordList[i].frequency < wordList[j].frequency) {
                WordFreq temp = wordList[i];
                wordList[i] = wordList[j];
                wordList[j] = temp;
            }
        }
 }

clock_t endTime = clock();
double timetaken = ((double)endTime - startTime) / CLOCKS_PER_SEC;
         printf("Top 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < wordcount; i++) {
        printf("%s: %d\n", wordList[i].word, wordList[i].frequency);
    }
 printf("Time taken to count and find the most frequent words: %.5f sec\n", timetaken);

return 0;
}
