#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "calcparser.h"


typedef enum { PRESERVE, CONSUME } ParseMode;
const char* map_SyntaxError[4]  = {
    "",
    "Error: Expected an operator next to a parenthesis.",
    "Error: Expected an integer.",
    "Error: Unbalanced parentheses."
};


/** EXPRESSION HANDLING */
typedef enum { LEFT, RIGHT } Associative;

typedef struct Math {
    TokenType type;
    Associative assoc;  // Left or right associative? 
    int priority;
    const char *str;
} Math;


typedef struct Stack {
    Math **data;
    Math  *top;  // Pointer to the next free element of the stack.
    int    size;
    int    max;   // Maximum stack size.
} Stack;


/**
 * Creates a new stack. Remember to free its resources when no longer needed.
 */
Stack* newStack(int maxSize) {
    Stack *stack = (Stack *) malloc(sizeof(Stack));
    stack->data = (Math **) calloc(maxSize, sizeof(Math *));
    stack->top = stack->data[0];
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
 * Returns true (1) if the stack is full, false (0) otherwise. 
 */
static int isFull(Stack *stack) {
    if (stack->top - 1 == stack->data[stack->max - 1])
        return 1;
    return 0;
}


/**
 * Returns true (1) if the stack is empty, false (0) otherwise. 
 */
static int isEmpty(Stack *stack) {
    if (stack->size == 0)
        return 1;
    return 0;
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


typedef struct Expression {
    Math *expr[128];  // Static for now.
    Math *end;
    int size;
} Expression;


Expression *newExpression() {
    Expression *e = (Expression *) malloc(sizeof(Expression));
    e->end = e->expr[0];
    e->size = 0;

    return e;
}


void freeExpression(Expression *e) {
    int x;
    for (x = 0; x < e->size; x++)
        free(e->expr[x]);
    free(e);
}


void addMath(Expression *e, TokenType type, Associative a, int priority, char *str) {
    Math *new = (Math *) malloc(sizeof(Math));
    new->type = type;
    new->assoc = a;
    new->priority = priority;
    new->str = str;
    e->expr[e->size++] = new; 
    e->end++;
}
/** EXPRESSION HANDLING */


/**
 * Converts infix expression 'e' to postfix.
 * URL: https://en.wikipedia.org/wiki/Shunting_yard_algorithm
 * 
 * The recursive descent parser ensured balanced parentheses.
 */
void shuntingYard(Expression *e) {
    Stack *operator = newStack(e->size);
    Math **output = (Math **) calloc(e->size, sizeof(Math *));
    int curExpr = 0;
    int curOut = 0;

    while ((e->expr[curExpr])->str != NULL) {
        Math *m = e->expr[curExpr];

        // A left parenthesis.
        if (strcmp(m->str, "(") == 0) {
            push(operator, m);
        }

        // A right parenthesis.
        else if (strcmp(m->str, ")") == 0) {
            while (strcmp(peek(operator)->str, "(") != 0) {
                output[curOut++] = pop(operator);

                if (strcmp(peek(operator)->str, "(") == 0)
                    pop(operator);
            }
        }
        // A number.
        else if (m->type == INTEGER)
            output[curOut++] = m;

        // A operator.
        else if (m->type == SYMBOL) {
            while (!isEmpty(operator)) {
                Math *p = peek(operator);
                if (strcmp(p->str, "(") != 0 &&
                        (p->priority < m->priority ||
                            (m->priority == p->priority && m->assoc == LEFT)))
                {
                    output[curOut++] = pop(operator);
                }
                else
                    break;
            }
            push(operator, m);
        }
        curExpr++;
    }

    while (!isEmpty(operator)) {
        output[curOut++] = pop(operator);
    }

    memcpy(e->expr, output, sizeof(Math **) * e->size);
    freeStack(operator);
    free(output);
}


/**
 * Evaluates postfix expression 'e'.  
 */
int eval(Expression *e) {

}


/** START PROTOTYPES */
void parse(char *str);
ParserInfo parseExpr();  // Start symbol
ParserInfo r_parseExpr(Expression *e);
ParserInfo parseTerm(Expression *e);
ParserInfo r_parseTerm(Expression *e);
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

    if (info->token == NULL || info->token->code == EOS) {
        info->error = error;
        return;
    }

    switch (error) {
        case NONE:
            return;

        case OPERAND:
            if (info->token->type != INTEGER)
                info->error = error;
            break;

        case PAREN_CLOSE:
            if (!(strcmp(info->token->lexeme, ")")) == 0)
                info->error = error;
            break;
    }
}


ParserInfo parseExpr(Expression *e) {
    ParserInfo info = {.error = NONE};

    info = parseTerm(e);
    if (info.error != NONE)
        return info;

    info = r_parseExpr(e);
    if (info.error != NONE)
        return info;

    return info;
}


ParserInfo r_parseExpr(Expression *e) {
    ParserInfo info = {.error = NONE};
    Token *t = peekNextToken();

    if (t == NULL || t->type == EOS)
        return info;

    if (strcmp(t->lexeme, "+") == 0|| strcmp(t->lexeme, "-") == 0) {
        addMath(e, SYMBOL, LEFT, 4, t->lexeme);
        getNextToken();

        info = parseTerm(e);
        if (info.error != NONE)
            return info;
        
        info = r_parseExpr(e);
        return info;
    }
    else if (strcmp(t->lexeme, "*") == 0 ||
             strcmp(t->lexeme, "/") == 0 ||
             strcmp(t->lexeme, ")") == 0)
    {
        return info;
    }

    info.error = OPERATOR;
    return info;
}


ParserInfo parseTerm(Expression *e) {
    ParserInfo info = {.error = NONE};

    info = parseFactor(e);
    if (info.error != NONE)
        return info;

    info = r_parseTerm(e);
    return info;
}


ParserInfo r_parseTerm(Expression *e) {
    ParserInfo info = {.error = NONE};
    Token *t = peekNextToken();

    if (t == NULL || t->type == EOS)
        return info;

    if (strcmp(t->lexeme, "*") == 0 || strcmp(t->lexeme, "/") == 0) {
        addMath(e, SYMBOL, LEFT, 3, t->lexeme);
        getNextToken();

        info = parseFactor(e);
        if (info.error != NONE)
            return info;
        
        info = r_parseTerm(e);
        return info;
    }
    else if (strcmp(t->lexeme, "+") == 0 ||
             strcmp(t->lexeme, "-") == 0 ||
             strcmp(t->lexeme, ")") == 0)
    {
        return info;
    }

    info.error = OPERATOR;
    return info;
}


ParserInfo parseFactor(Expression *e) {
    ParserInfo info = {.error = NONE};

    if (strcmp(peekNextToken()->lexeme, "(") == 0) {
        addMath(e, SYMBOL, LEFT, 1, "(");
        
        getNextToken();
        info = parseExpr(e);
        if (info.error != NONE)
            return info;

        checkFor(PAREN_CLOSE, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, SYMBOL, LEFT, 1, ")");
    }
    else if (strcmp(peekNextToken()->lexeme, "-") == 0) {
        addMath(e, SYMBOL, RIGHT, 2, "-");  // Unary minus.
        
        getNextToken();
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, INTEGER, LEFT, 0, info.token->lexeme);
    }
    else {
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, INTEGER, LEFT, 0, info.token->lexeme);
    }
    return info;
}


void parse(char *str) {
    if (!initLexer(str)) {
        stopLexer();
        return;
    }

    Expression *expr = newExpression();
    ParserInfo info = parseExpr(expr);

    // Check for any extra/illegal parentheses outside of an expression.
    if (peekNextToken() != NULL &&
        strcmp(peekNextToken()->lexeme, ")") == 0)
    {
        info.error = PAREN_CLOSE;
    }

    if (info.error != NONE) {
        printf("%s\n", map_SyntaxError[info.error]);
        return;
    }

    shuntingYard(expr);  // Infix -> Postfix (RPN)
    int result = eval(expr);
    stopLexer();
}
