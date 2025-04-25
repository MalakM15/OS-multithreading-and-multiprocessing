//Malak Milhem   1220031

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#define MAXWORDLENGTH 50
#define NUMOFWORDS 300000

typedef struct{
    char word[MAXWORDLENGTH];
    int frequency;
}WordFreq;


int totalwordcount = 0;  //num of unique words
 WordFreq wordList [NUMOFWORDS];
 pthread_mutex_t mergeMutex;

typedef struct {
    char *startofpart;   // start position in the file
    long partsize;        // size of the file part
    WordFreq *localList; 
    int threadCount;
    } Threadstruct; 
//compare by freq
int compare(const void *a, const void *b) {
    return ((WordFreq*)b)->frequency - ((WordFreq*)a)->frequency;
}
int findWord(WordFreq *list, int wordcount,const char *word){
    for (int i =0; i<wordcount ; i++){
        if (strcmp(list[i].word, word) == 0){
            return i; //the word's index
        }
    }
    return -1 ; //word not found
}

void insertWord (WordFreq *list, int *wordcount,const char *word){
    int index = findWord (list , *wordcount, word);
    if (index != -1){
    list[index].frequency++;
    } else {
        strcpy(list[*wordcount].word,word);
         list[*wordcount].frequency = 1;
        (*wordcount)++;
    }
}

void mergeLists(WordFreq *localList, int threadCount){
     pthread_mutex_lock(&mergeMutex);
    for (int i = 0; i < threadCount; i++) {
        int index = findWord(wordList, totalwordcount, localList[i].word);
        if (index != -1) {
            // Word exists add frequency
            wordList[index].frequency += localList[i].frequency;
        } else {
            //no such word insert new word
            strcpy(wordList[totalwordcount].word, localList[i].word);
            wordList[totalwordcount].frequency = localList[i].frequency;
            totalwordcount++;
        }
    }
    pthread_mutex_unlock(&mergeMutex);
}

void *threadscount(void *arg){  //thread function
    Threadstruct *data = (Threadstruct *)arg;
    char word[MAXWORDLENGTH];
    char *endofpart = data->startofpart + data->partsize;
    char *currentPos = data->startofpart;
    data->threadCount = 0;
    int wordIndex= 0;
    while(currentPos < endofpart){
        wordIndex= 0;
        while (currentPos < endofpart && *currentPos != ' '){
            if(wordIndex < MAXWORDLENGTH - 1) {
                word[wordIndex++] = *currentPos;
            }
            currentPos++;
        }
        word[wordIndex] = '\0';
        if (strlen(word) > 0) {
            insertWord(data->localList, &data->threadCount, word);
        }
          while (currentPos < endofpart && (*currentPos == ' ' || *currentPos == '\n')) {
            currentPos++;
        }
        }
     mergeLists (data->localList, data->threadCount);
        return NULL;
    }

int main(){
FILE *file = fopen("/mnt/c/Users/user/Downloads/text8.txt", "r");
    if (file == NULL) {
        perror("Failed to open file !! ");
        return 1;
    }

 struct timeval start, end;
double timetaken;
pthread_t *threads;

int numThreads;  
printf ("Enter num of threads wanted: ");
scanf("%d", &numThreads);
Threadstruct *ThreadsInfo;


fseek(file, 0, SEEK_END);
long fileSize = ftell(file);  // to get the file size
fseek(file, 0, SEEK_SET);// rewind(file);


threads = malloc(numThreads * sizeof(pthread_t));
if (threads == NULL) {
        perror("Fail allocating memory for threads");
        fclose(file);
        return 1;
    }
ThreadsInfo = malloc(numThreads * sizeof(Threadstruct));
if (ThreadsInfo == NULL) {
    perror("Failed to allocate memory for ThreadsInfo");
    free(threads);
    fclose(file);
    return 1;
}
long textSize = fileSize / numThreads;
pthread_mutex_init(&mergeMutex,NULL);
gettimeofday(&start,NULL);

for (int i = 0; i < numThreads; i++) {
    ThreadsInfo[i].partsize = textSize;
        ThreadsInfo[i].threadCount = 0;
    
        if (i == numThreads - 1) {
            ThreadsInfo[i].partsize += fileSize % numThreads;  // last thread takes the remaning part
        } 
        ThreadsInfo[i].startofpart = calloc(ThreadsInfo[i].partsize, sizeof(char));
          if (ThreadsInfo[i].startofpart == NULL) {
            perror("Failed to allocate memory for startofpart");
            fclose(file);
            free(threads);
            free(ThreadsInfo);
            return 1;
        }
           ThreadsInfo[i].localList = calloc(NUMOFWORDS, sizeof(WordFreq));
    if (ThreadsInfo[i].localList == NULL) {
        perror("Failed to allocate memory for localList");
        free(ThreadsInfo[i].startofpart);
        fclose(file);
        free(threads);
        free(ThreadsInfo);
        return 1;
    }
         fseek(file, i * textSize, SEEK_SET);  
         //####
         fread(ThreadsInfo[i].startofpart, sizeof(char), ThreadsInfo[i].partsize, file);
//size_t bytesRead = fread(ThreadsInfo[i].startofpart, sizeof(char), ThreadsInfo[i].partsize, file);
//if (bytesRead < ThreadsInfo[i].partsize) {
//    if (feof(file)) {
//        fprintf(stderr, "Unexpected end of file.\n");
//    } else if (ferror(file)) {
//        perror("Error reading file");
//    }
//    free(ThreadsInfo[i].startofpart);
//    fclose(file);
//    free(threads);
//    free(ThreadsInfo);
//    return 1;
//}
//####
        pthread_create(&threads[i], NULL, threadscount, &ThreadsInfo[i]);
    }
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL); // waiting for all threads
        free(ThreadsInfo[i].startofpart);
    }

     fclose(file);

  // sort final list of words
  qsort(wordList, totalwordcount, sizeof(WordFreq), compare);

gettimeofday(&end,NULL);
timetaken=(end.tv_sec - start.tv_sec) * 1000000.0;
timetaken+= (end.tv_usec - start.tv_usec);
         printf("Top 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < totalwordcount; i++) {
        printf("%s: %d\n", wordList[i].word, wordList[i].frequency);
    }
 printf("Time taken : %.5f sec\n", timetaken/1000000.0);
 free(threads);
 free(ThreadsInfo);
 
 pthread_mutex_destroy(&mergeMutex);

return 0;
}