#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenize.h"

const char SYMBOLS[6] = {'+', '-', '*', '/', '(', ')'};

const char* map_TokenType[4] = { "INTEGER", "SYMBOL", "EOS", "ERROR" };
const char* map_LexError[1]  = { "Error: illegal symbol in input" };


char* dupstr(const char *src) {
    int len = strlen(src) + 1;
    char *s = (char *) malloc(sizeof(char) * len);

    if (s == NULL)
        return NULL;
    return (char *) memcpy(s, src, len);
}


/**
 * Wrapper for singly linked list tokens.
 */
typedef struct LinkedList {
    Token *start;  // Pointer to head of list.
    Token *end;    // Pointer to end of list.
    Token *cur;    // Pointer to a token of interest (for Get/Peek token functions).
    int    size;   // List size (number of tokens).
} LinkedList;


/**
 * Creates a new list. Remember to free its resources when no longer needed.
 */
LinkedList* newLinkedList() {
    LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));
    list->start = NULL;
    list->end = NULL;
    list->cur = NULL;
    list->size = 0;

    return list;
}


Token* createToken(TokenType type, char *lexeme, LexError error) {
    Token *t = (Token *) malloc(sizeof(Token));
    t->lexeme = dupstr(lexeme);
    t->code = error;
    t->type = type;

    return t;
}


void freeToken(Token *t) {
    free(t->lexeme);
    free(t);
}


/**
 * Returns true (1) if the list is empty, false (0) otherwise. 
 */
static int isEmpty(LinkedList *list) {
    if (list->size == 0 || list->start == NULL)
        return 1;
    return 0;
}


/**
 * Frees all memory dynamically allocated to the list, including its tokens. 
 */
void freeLinkedList(LinkedList *list) {
    if (isEmpty(list)) {
        free(list);
        return;
    }

    Token *iterator = list->start;
    Token *next;

    while (1) {
        next = iterator->next;
        freeToken(iterator);

        if (next == NULL)
            break;
        iterator = next;
    }
    free(list);
}


/**
 * Adds the token to the end of the list.
 */
void insertEnd(LinkedList *list, Token *new) {
    if (list->start != NULL) {
        // Adding a token to the end of a non-empty list.
        Token *end = list->end;
        end->next = new;
        new->next = NULL;
        list->end = new;
    }
    else {
        // Adding a token to the list if it is empty.
        list->start = new;
        list->end = new;
        list->cur = new;
        new->next = NULL;
    }

    // Increase list size, we have added a token.
    list->size++;
}

/** END LINKEDLIST */


/**
 * Simple lexer for the calculator application. 
 */
void printToken(Token *t) {
    printf("< %s, %s >\n", 
            t->lexeme, 
            map_TokenType[t->type]
    );
}


int isSymbol(char c) {
    int x;
    for (x = 0; x < (sizeof(SYMBOLS) / sizeof(SYMBOLS[0])); x++) {
        if (c == SYMBOLS[x])
            return 1;
    }
    return 0;
}


// List of tokens read during tokenization.
static LinkedList *TOKENS;


Token* tokenize(char *str) {
    Token *token;

    int cur = 0;
    char c = str[cur];
    char temp[128];

    if (isdigit(c)) {
        // This is the first character of an integer constant.
        int i = 0;
        while (isdigit(c)) {
            temp[i++] = c;
            c = str[++cur];
        }
        temp[i] = '\0';

        token = createToken(INTEGER, temp, NO_ERROR);
        return token;
    }

    // The character must be a symbol.
    if (isSymbol(c)) {
        temp[0] = c;
        temp[1] = '\0';
        token = createToken(SYMBOL, temp, NO_ERROR);
        return token;
    }

    // The character might be the end of a string.
    if (c == '\0') {
        temp[0] = '\0';
        token = createToken(EOS, temp, NO_ERROR);
        return token;
    }

    // The character must be illegal.
    temp[0] = c;
    temp[1] = '\0';
    token = createToken(ERROR, temp, ILLEGAL_SYMBOL);
    return token;
}


int initLexer(char *input) {
    int len = 0;
    TOKENS = newLinkedList();

    do {
        Token *t = tokenize(input + len);
        printToken(t);
        insertEnd(TOKENS, t);

        if (t->code == ILLEGAL_SYMBOL) {
            printf("%s '%s'\n", map_LexError[t->code], t->lexeme);
            return 0;
        }
        else if (t->type == EOS)
            break;

        len += strlen(t->lexeme);
    } while (1);

    return 1;
}


void stopLexer() {
    freeLinkedList(TOKENS);
}


/**
 * Get the next token from the CLI and remove it from the stream. 
 */
Token* getNextToken() {
    Token *t;

    if (TOKENS->cur == TOKENS->start)
        t = TOKENS->start;
    else
        t = TOKENS->cur;

    TOKENS->cur = t->next;
    return t;
}


/**
 * Returns the next token in the source file without removing it from the stream.
 */
Token* peekNextToken() {
    if (TOKENS->cur == TOKENS->start)
        return TOKENS->start;

    return TOKENS->cur;
}
