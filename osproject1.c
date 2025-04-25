//Malak Milhem   1220031

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 100
#define TOP_WORDS 10

typedef struct {    
    char word[MAX_WORD_LENGTH];
    int frequency;
} WordFreq;
typedef struct TreeNode {   //tree node structure
    char word[MAX_WORD_LENGTH];
    int frequency;
    struct TreeNode *left, *right;
} TreeNode;

TreeNode* createNode(char *word) {
    TreeNode* newNode = (TreeNode*) malloc(sizeof(TreeNode));
    strcpy(newNode->word, word);
    newNode->frequency = 1;  // first occurrence of the word
    newNode->left = newNode->right = NULL;
    return newNode;
}

TreeNode* insert(TreeNode *root, char *word) { // insert a word to the tree
    if (root == NULL) {
        return createNode(word);
    }

    int cmp = strcmp(word, root->word);
    if (cmp < 0) {
        root->left = insert(root->left, word);
    } else if (cmp > 0) {
        root->right = insert(root->right, word);
    } else {
        root->frequency++;  // word found before increment frequency
    }
    return root;
}
///left-root-right traversal
void inorder(TreeNode *root, WordFreq **wordList, int *index, int *capacity) {
    if (root != NULL) {
        inorder(root->left, wordList, index, capacity);

   
        if (*index >= *capacity) { //  the list is full resize it
            *capacity *= 2;  // Double the capacity
            *wordList = realloc(*wordList, *capacity * sizeof(WordFreq));
            if (*wordList == NULL) {
                perror("Reallocation failed");
                exit(1);
            }
        }
        // add word and its frequency to the wordList
        strcpy(wordList[*index].word, root->word);
        wordList[*index].frequency = root->frequency;
        (*index)++;
        inorder(root->right, wordList, index, capacity);
    }
}

//compare by freq
int compare(const void *a, const void *b) {
    return ((WordFreq*)b)->frequency - ((WordFreq*)a)->frequency;
}

int main() {
    printf("Hello, World!\n");

    FILE *file = fopen("C:\Users\user\Downloads\text8.txt","r");
     if (file == NULL) {
        perror("can't open file");
        return 1;
    }
    TreeNode *root = NULL;
    char word[MAX_WORD_LENGTH];

    // Insert words into the tree
    while (fscanf(file, "%s", word) != EOF) {
        root = insert(root, word);  // Insert each word into the tree
    }
    fclose(file);

    int capacity = 10000;  // start with capacity 10,000 words
    WordFreq *wordList = malloc(capacity * sizeof(WordFreq));
    if (wordList == NULL) {
        perror("Memory allocation failed");
        return 1;
    } 
     int index = 0;

    // in-order traversal and store the words and frequencies in the list
    inorder(root, &wordList, &index, &capacity);

    // sort the list by frequency
    qsort(wordList, index, sizeof(WordFreq), compare);

    // Print the top 10 most frequent words
    printf("Top 10 frequent words:\n");
    for (int i = 0; i < (index < TOP_WORDS ? index : MAX_TOP_WORDS); i++) {
        printf("%s: %d\n", wordList[i].word, wordList[i].frequency);
    }

    free(wordList);
    free(root);  //free memory

    return 0;
}