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
    struct Math *next;
} Math;


typedef struct Expression {
    Math *start; // Pointer to head of list.
    Math *end;   // Pointer to end of list.
    Math *cur;
    int   size;  // List size (number of nodes).
} Expression;


/**
 * Creates a new list. Remember to free its resources when no longer needed.
 */
Expression* newExpression() {
    Expression *expr = (Expression *) malloc(sizeof(Expression));
    expr->start = NULL;
    expr->end = NULL;
    expr->cur = NULL;
    expr->size = 0;

    return expr;
}


Math* newMath(TokenType t, Associative a, int p, char *s) {
    Math *m = (Math *) malloc(sizeof(Math));
    m->type = t;
    m->assoc = a;
    m->priority = p;
    m->str = s;
    m->next = NULL;

    return m;
}


void freeMath(Math *m) {
    // Free the node itself and any of its members allocated memory.
    free(m);
}


void freeExpression(Expression *e) {
    if (isEmpty(e)) {
        free(e);
        return;
    }

    Math *iterator = e->start;
    Math *next;

    while (1) {
        next = iterator->next;
        freeMath(iterator);

        if (next == NULL)
            break;
        iterator = next;
    }
    free(e);
}


void addMath(Expression *e, TokenType t, Associative a, int priority, char *str) {
    Math *new = newMath(t, a, priority, str);

    if (e->start != NULL) {
        // Adding math to a non-empty expression.
        Math *end = e->end;
        end->next = new;
        new->next = NULL;
        e->end = new;
    }
    else {
        // Adding math to an empty expression.
        e->start = new;
        e->end = new;
        e->cur = new;
        new->next = NULL;
    }

    // Increase list size, we have added a token.
    e->size++;
}


Math* getNextMath(Expression *e) {
    Math *m;

    if (e->cur == e->start)
        m = e->start;
    else
        m = e->cur;

    e->cur = m->next;
    return m;
}


int exprEmpty(Expression *e) {
    if (e->size == 0 || e->start == NULL)
        return 1;
    return 0;
}


/**
 * Prints the expression.
 */
void printExpr(Expression *e) {
    if (exprEmpty(e)) {
        printf("----------------------\n");
        printf("The expression is empty\n");
        printf("----------------------\n\n");
        return;
    }
    
    Math *iterator = e->start;
    while (1) {
        printf("----------------------\n");
        printf("address:  %p\n", iterator);
        printf("data:     %d\n", iterator->str);
        printf("next:     %p\n", iterator->next);
        printf("----------------------\n\n");

        if (iterator->next == NULL)
            break;
        iterator = iterator->next;
    }

    printf("Size of list: %d\n",   e->size);
    printf("Start:        %p\n",   e->start);
    printf("End:          %p\n\n", e->end);
}
/** EXPRESSION HANDLING */


/**
 * Converts infix expression 'e' to postfix.
 * URL: https://en.wikipedia.org/wiki/Shunting_yard_algorithm
 * 
 * The recursive descent parser ensured balanced parentheses.
 */
void shunt(Expression *e) {

}


/**
 * Evaluates postfix expression 'e'.  
 */
int eval(Expression *e) {

}


/** START PROTOTYPES */
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

    Expression *expr = newExpression(128);
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

    shunt(expr);  // Infix -> Postfix (RPN)
    int result = eval(expr);
    stopLexer();
}
