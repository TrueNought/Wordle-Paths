#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordle.h"
#include "constraints.h"

/* Read the wordle grid and solution from fp. 
 * Return a pointer to a wordle struct.
 * See sample files for the format. Assume the input file has the correct
 * format.  In other words, the word on each is the correct length, the 
 * words are lower-case letters, and the line ending is either '\n' (Linux,
 * Mac, WSL) or '\r\n' (Windows)
 */
struct wordle *create_wordle(FILE *fp) {
    struct wordle *w = malloc(sizeof(struct wordle));
    char line[MAXLINE]; 
    w->num_rows = 0;

    while(fgets(line, MAXLINE, fp ) != NULL) {
        
        // remove the newline character(s) 
        char *ptr;
        if(((ptr = strchr(line, '\r')) != NULL) ||
           ((ptr = strchr(line, '\n')) != NULL)) {
            *ptr = '\0';
        }
        
        strncpy(w->grid[w->num_rows], line, SIZE);
        w->grid[w->num_rows][SIZE - 1] = '\0';
        w->num_rows++;
    }
    return w;
}


/* Create a solver_node and return it.
 * If con is not NULL, copy con into dynamically allocated space in the struct
 * If con is NULL set the new solver_node con field to NULL.
 * Tip: struct assignment makes copying con a one-line statements
 */
struct solver_node *create_solver_node(struct constraints *con, char *word) {

    // TODO - replace return statement when function is implemented
    struct solver_node *s = malloc(sizeof(struct solver_node));
    
    strcpy(s->word, word);
    (s->word)[5] = '\0';
    s->next_sibling = NULL;
    s->child_list = NULL;
    
    if (con != NULL) {
        s->con = con;
    }
    else {
        s->con = NULL;
    }
    return s;
}

/* Returns a string containing only the letters that are in the cannot_be set
 *
 */
char *get_cannot_be(struct constraints *con) {
    char *cb = malloc(ALPHABET_SIZE+1);
    int curr = 0;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if ((con->cannot_be)[i] == 1) {
            cb[curr] = 97 + i;
            curr++;     
        }
    }  
    cb[curr] = '\0';
    return cb;
}

/* Checks any duplicate characters in the guess word against the solution word so that
 * our guess does not contain duplicate solution word letters
 */ 
int check_solution_duplicates(char letter, char *solution) {
    for (int i = 0; i < WORDLEN; i++) {
        if (solution[i] == letter) {
            return 1;
        }
    }
    return 0;
}

/* Return 1 if "word" matches the constraints in "con" for the wordle "w".
 * Return 0 if it does not match
 */
int match_constraints(char *word, struct constraints *con, 
struct wordle *w, int row) {

    // TODO
    char *cb = get_cannot_be(con);

    for (int i = 0; i < WORDLEN; i++) {
        // If must_be[index] is not the empty string
        if (con->must_be[i][0] != '\0') {
            // Does not match if word[index] is not in must_be[index] string
            if (!strchr((con->must_be)[i], word[i])) {
                free(cb);
                return 0;
            }
            // Does not match if letter at word[index] matches letter at index in solution word 
            // (case when the guess result is yellow, i.e. must_be[index] contains > 1 letter)
            if ((con->must_be)[i][1] != '\0') {
                if (word[i] == w->grid[0][i]) {
                    free(cb);
                    return 0;
                }
            }
        }
        // Case if must_be[index] is the empty string
        else {
            // Does not match if word[index] is in cannot_be set
            if (strchr(cb, word[i])) {
                free(cb);
                return 0;
            }
        }
        // Ensure guess does not contain duplicate solution word letters
        for (int j = i+1; j < WORDLEN; j++) {
            if (word[i] == word[j]) {
                if (check_solution_duplicates(word[i], w->grid[0]) == 1) {
                    free(cb);
                    return 0;
                }
            }
        }
    }
    free(cb);
    return 1;
}

/* remove "letter" from "word"
 * "word" remains the same if "letter" is not in "word"
 */
void remove_char(char *word, char letter) {
    char *ptr = strchr(word, letter);
    if(ptr != NULL) {
        *ptr = word[strlen(word) - 1];
        word[strlen(word) - 1] = '\0';
    }
}

/* Resets the must_be constraint when we call solver_tree on another word
 */
void reset_must_be(struct solver_node *n) {
    for (int i = 0; i < WORDLEN; i++) {
        (n->con->must_be)[i][0] = '\0';
    }
}

/* Build a tree starting at "row" in the wordle "w". 
 * Use the "parent" constraints to set up the constraints for this node
 * of the tree
 * For each word in "dict", 
 *    - if a word matches the constraints, then 
 *        - create a copy of the constraints for the child node and update
 *          the constraints with the new information.
 *        - add the word to the child_list of the current solver node
 *        - call solve_subtree on newly created subtree
 */

void solve_subtree(int row, struct wordle *w,  struct node *dict, 
                   struct solver_node *parent) {
    if(verbose) {
        printf("Running solve_subtree: %d, %s\n", row, parent->word);
    }

    // TODO
    // Base case if we reach the end of the wordle input
    if (row == w->num_rows) {
        parent->con = NULL;
        parent->child_list = NULL;
        return;
    }
    // Make or update the constraints
    char cannot[SIZE];
    strcpy(cannot, parent->word);
    cannot[WORDLEN] = '\0';

    for (int i = 0; i < WORDLEN; i++) {
        if ((w->grid)[row][i] == 'g') {
            set_green((parent->word)[i], i, parent->con);
        }
        else if ((w->grid)[row][i] == 'y') {
            // If this is the initial root of the solver tree
            if (row == 1) {
                set_yellow(i, (w->grid)[row], "ggggg", (w->grid)[0], parent->con);
            }
            else {
                set_yellow(i, (w->grid)[row], (w->grid)[row-1], parent->word, parent->con);
            }
        }
    }
    // We can just add all the letters in the word to cannot_be constraint
    add_to_cannot_be(cannot, parent->con);

    // Going through the dictionary
    // We make a pointer to the start of the dictionary so we can keep track of traversal
    int first = 0;
    struct solver_node *curr;
    struct node *ptr = dict;
    while (ptr) {
        if (match_constraints(ptr->word, parent->con, w, row) == 1) {
            // Create a copy of the constraints for the child node
            struct constraints *new_con = malloc(sizeof(struct constraints));
            *new_con = *(parent->con);
            
            // If this word is the first match
            if (first == 0) {        
                curr = create_solver_node(new_con, ptr->word);  
                reset_must_be(curr);
                parent->child_list = curr;
                first = 1;
                solve_subtree(row + 1, w, dict, curr);

            }
            else {
                struct solver_node *match = create_solver_node(new_con, ptr->word);
                reset_must_be(match);
                curr->next_sibling = match;
                curr = match; 
                solve_subtree(row + 1, w, dict, match);         
            }
            free(new_con);
        }
        ptr = ptr->next;
    }

    // debugging suggestion, but you can choose how to use the verbose option
    /*if(verbose) {
        print_constraints(c);
    } */

    // TODO
}

/* Print to standard output all paths that are num_rows in length.
 * - node is the current node for processing
 * - path is used to hold the words on the path while traversing the tree.
 * - level is the current length of the path so far.
 * - num_rows is the full length of the paths to print
 */

void print_paths(struct solver_node *node, char **path, 
                 int level, int num_rows) {

    // TODO
    // Base case: if we have reached the end then we print the path
    if (level == num_rows) {
        path[level-1] = node->word;
        printf("%s", path[0]);
        for (int i = 1; i < num_rows; i++) {
            printf(" %s", path[i]);
        }
        printf("\n");
    }
    else if (level < num_rows && (node->child_list) == NULL) {
        return;
    }
    else {
        struct solver_node *curr = node->child_list;
        path[level-1] = node->word;
        while (curr) {       
            print_paths(curr, path, level+1, num_rows);
            curr = curr->next_sibling;
        }
    }
}

/* Free all dynamically allocated memory pointed to from w.
 */ 
void free_wordle(struct wordle *w){
    // TODO
    free(w);
}

/* Free all dynamically allocated pointed to from node
 */
void free_tree(struct solver_node *node){
    // TODO
    if (!(node->child_list) && !(node->next_sibling)) {
        free(node);
    }
    else {
        struct solver_node *curr = node->child_list;
        while (curr) {
            struct solver_node *next = curr->next_sibling;
            free_tree(curr);
            curr = next;
        }
        free(node);
    }
}
