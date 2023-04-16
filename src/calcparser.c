#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tokenize.h"
#include "calcparser.h"


typedef enum { PRESERVE, CONSUME } ParseMode;
const char* map_SyntaxError[5]  = {
    "",
    "Error: Expected a '+', '-', '*' or '/'.",
    "Error: Expected an integer.",
    "Error: Unbalanced parentheses."
};


/** EXPRESSION HANDLING */
typedef struct Stack {
    Math **data;
    Math *top;  // Pointer to the next free element of the stack.
    int size;
    int max;   // Maximum stack size.
} Stack;


/**
 * Creates a new stack. Remember to free its resources when no longer needed.
 */
Stack* newStack(int maxSize) {
    Stack *stack = (Stack *) malloc(sizeof(Stack));
    stack->data = (Math **) calloc(maxSize, sizeof(Math *));
    stack->top = stack->data;
    stack->max = maxSize;
    stack->size = 0;

    return stack;
}


/**
 * Frees all memory dynamically allocated to the stack. 
 */
void freeStack(Stack *stack) {
    free(stack->data);
    free(stack);
}


/**
 * Push (add) 'data' onto the top of the stack.
 * Returns if the operation would cause a stack overflow.
 */
void push(Stack *stack, Math *data) {
    if (isFull(stack))
        return;

    stack->top = data;
    stack->top++;
    stack->size++;
}


/**
 * Pop (remove) an element from the top of the stack.
 * Returns 0 if the operation would cause a stack underflow.
 */
Math* pop(Stack *stack) {
    if (isEmpty(stack))
        return 0;

    Math *element = --stack->top;
    stack->size--;

    return element;
}


/**
 * Peek (inspect) the top element of the stack. 
 */
Math* peek(Stack *stack) {
    return stack->top - 1;
}


/**
 * Returns true (1) if the stack is full, false (0) otherwise. 
 */
int isFull(Stack *stack) {
    if (stack->top - 1 == stack->data + stack->max - 1)
        return 1;
    return 0;
}


/**
 * Returns true (1) if the stack is empty, false (0) otherwise. 
 */
int isEmpty(Stack *stack) {
    if (stack->top == stack->data)
        return 1;
    return 0;
}


/**
 * Prints the stack. 
 */
void printStack(Stack *stack) {
    if (isEmpty(stack)) {
        printf("----------------------\n");
        printf("The stack is empty\n");
        printf("----------------------\n\n");
        return;
    }

    int x;
    printf("------Stack Top------\n");
    for (x = stack->size - 1; x >= 0; x--) {
        printf("%s\n", *(stack->data[x])->str);
    }
    printf("-----Stack Bottom-----\n\n");
}


typedef struct Math {
    TokenType type;
    int priority;
    const char *str;
} Math;


typedef struct Expression {
    Math expr[50];  // Static for now.
    int cur;
} Expression;


Expression *newExpression() {
    Expression *e = (Expression *) malloc(sizeof(Expression));
    e->cur = 0;

    return e;
}


void freeExpression(Expression *e) {
    free(e);
}


void addMath(Expression *e, TokenType type, int priority, char *str) {
    e->expr[e->cur++] = (Math) {type, priority, str};
}
/** EXPRESSION HANDLING */


/**
 * Converts infix expression 'e' to postfix. 
 */
void shuntingYard(Expression *e) {

}


/**
 * Evaluates postfix expression 'e'.  
 */
int eval(Expression *e) {

}


/** START PROTOTYPES */
void parseExpr();  // Start symbol
ParserInfo r_parseExpr(Expression *e);
ParserInfo parseAdd(Expression *e);
ParserInfo parseTerm(Expression *e);
ParserInfo r_parseTerm(Expression *e);
ParserInfo parseMult(Expression *e);
ParserInfo parseFactor(Expression *e);
/** END PROTOTYPES */


void checkFor(SyntaxError error, ParserInfo *info, ParseMode mode) {
    switch (mode) {
        case CONSUME:
            info->token = getNextToken();
            break;

        case PRESERVE:
            info->token = peekNextToken();
            break;
    }
    info->error = NONE;

    switch (error) {
        case NONE:
            return;

        case OPERAND:
            if (info->token->type != INTEGER)
                info->error = OPERAND;
            break;

        case OPERATOR:
            if (!(strcmp(info->token->lexeme, SYMBOLS[PLUS]) ||
                  strcmp(info->token->lexeme, SYMBOLS[MINUS]) ||
                  strcmp(info->token->lexeme, SYMBOLS[MULT]) ||
                  strcmp(info->token->lexeme, SYMBOLS[DIV])))
            {
                info->error = OPERATOR;
            }
            break;

        case PAREN_CLOSE:
            if (!strcmp(info->token->lexeme, SYMBOLS[CLOSE]))
                info->error = PAREN_CLOSE;
            break;
    }
}


void parseExpr() {
    ParserInfo info = {.error = NONE};
    Expression *expr = newExpression();

    info = parseTerm(expr);
    if (info.error != NONE)
        return info;

    info = r_parseExpr(expr);
    if (info.error != NONE)
        printf("%s\n", map_SyntaxError[info.error]);
    
    shuntingYard(expr);  // Infix -> Postfix (RPN)
    printf("= %d", eval(expr));
}


ParserInfo r_parseExpr(Expression *e) {
    ParserInfo info = {.error = NONE};

    if (peekNextToken()->code == EOS)
        return info;

    info = parseAdd(e);
    if (info.error != NONE)
        return info;

    info = parseTerm(e);
    if (info.error != NONE)
        return info;

    info = r_parseExpr(e);
    return info;
}


ParserInfo parseAdd(Expression *e) {
    ParserInfo info = {.error = NONE};

    checkFor(OPERATOR, &info, CONSUME);
    if (info.error != NONE)
        return info;

    if (strcmp(info.token->lexeme, SYMBOLS[PLUS])) {
        addMath(e, SYMBOL, 4, SYMBOLS[PLUS]);
    }
    else if (strcmp(info.token->lexeme, SYMBOLS[MINUS])) {
        addMath(e, SYMBOL, 4, SYMBOLS[MINUS]);
    }
    else
        info.error = OPERATOR;

    return info;
}


ParserInfo parseMult(Expression *e) {
    ParserInfo info = {.error = NONE};

    checkFor(OPERATOR, &info, CONSUME);
    if (info.error != NONE)
        return info;

    if (strcmp(info.token->lexeme, SYMBOLS[MULT])) {
        addMath(e, SYMBOL, 3, SYMBOLS[MULT]);
    }
    else if (strcmp(info.token->lexeme, SYMBOLS[DIV])) {
        addMath(e, SYMBOL, 3, SYMBOLS[DIV]);
    }
    else
        info.error = OPERATOR;

    return info;
}


ParserInfo parseFactor(Expression *e) {
    ParserInfo info = {.error = NONE};

    if (strcmp(peekNextToken()->lexeme, "(")) {
        addMath(e, SYMBOL, 1, SYMBOLS[OPEN]);
        
        getNextToken();
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE) {
            addMath(e, INTEGER, 0, info.token->lexeme);
        }
        else
            return info;

        checkFor(PAREN_CLOSE, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, SYMBOL, 1, SYMBOLS[CLOSE]);
    }
    else if (strcmp(peekNextToken()->lexeme, "-")) {
        addMath(e, SYMBOL, 2, SYMBOLS[MINUS]);  // Unary
        
        getNextToken();
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, INTEGER, 0, info.token->lexeme);
    }
    else {
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, INTEGER, 0, info.token->lexeme);
    }

    return info;
}
