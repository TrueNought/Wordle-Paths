#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordlist.h"


/* Read the words from a filename and return a linked list of the words.
 *   - The newline character at the end of the line must be removed from
 *     the word stored in the node.
 *   - You an assume you are working with Linux line endings ("\n").  You are
 *     welcome to also handle Window line endings ("\r\n"), but you are not
 *     required to, and we will test your code on files with Linux line endings.
 *   - The time complexity of adding one word to this list must be O(1)
 *     which means the linked list will have the words in reverse order
 *     compared to the order of the words in the file.
 *   - Do proper error checking of fopen, fclose, fgets
 */
struct node *read_list(char *filename) {
    // TODO - Remember to update return statement
    FILE *words_file;
    int error;
    char line[SIZE];
    int check = 0;

    // Opening file
    words_file = fopen(filename, "r");
    if (words_file == NULL) {
        fprintf(stderr, "Error opening input file\n");
        exit(1);
    }

    struct node *curr = malloc(sizeof(struct node));
    struct node *prev = curr;

    while (fgets(line, SIZE+1, words_file) != NULL) {
        line[SIZE-1] = '\0';

        if (check == 0) {
            strncpy(curr->word, line, SIZE);
            curr->next = NULL;
            check = 1;
        }
        else {
            curr = malloc(sizeof(struct node));
            strncpy(curr->word, line, SIZE);
            curr->next = prev;
            prev = curr;
        }
    }

    // Closing file
    error = fclose(words_file);
    if (error != 0) {
        fprintf(stderr, "fclose failed on input file\n");
        exit(1);
    }

    return curr;
}

/* Print the words in the linked-list list one per line
 */
void print_dictionary(struct node *list) {
    // TODO
    while (list != NULL) {
        printf("%s\n", list->word);
        list = list->next;
    }
}
/* Free all of the dynamically allocated memory in the dictionary list 
 */
void free_dictionary(struct node *list) {
    // TODO
    struct node *temp;

    while (list->next != NULL) {
        temp = list->next;
        free(list);
        list = temp;
    }
    free(list);
}

