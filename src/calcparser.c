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
    int value;  // Cast to char if type == SYMBOL.
    struct Math *next;
} Math;


typedef struct Expression {
    Math *start; // Pointer to head of list.
    Math *end;   // Pointer to end of list.
    Math *cur;
    int   size;  // List size (number of nodes).
} Expression;


Expression* newExpression() {
    Expression *expr = (Expression *) malloc(sizeof(Expression));
    expr->start = NULL;
    expr->end = NULL;
    expr->cur = NULL;
    expr->size = 0;

    return expr;
}


Math* newMath(TokenType t, Associative a, int p, int v) {
    Math *m = (Math *) malloc(sizeof(Math));
    m->type = t;
    m->assoc = a;
    m->priority = p;
    m->value = v;
    m->next = NULL;

    return m;
}


int exprEmpty(Expression *e) {
    if (e->size == 0 || e->start == NULL)
        return 1;
    return 0;
}


void freeMath(Math *m) {
    // Free the node itself and any of its members allocated memory.
    free(m);
}


void freeExpression(Expression *e) {
    if (exprEmpty(e)) {
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


void addMath(Expression *e, TokenType t, Associative a, int priority, int v) {
    Math *new = newMath(t, a, priority, v);

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

    if (e->cur == NULL)
        return NULL;

    else if (e->cur == e->start)
        m = e->start;
    else
        m = e->cur;

    e->cur = m->next;
    return m;
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
        if (iterator->type == SYMBOL)
            printf("data:     %c\n", (char) iterator->value);
        else
            printf("data:     %d\n", iterator->value);
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


typedef struct Stack {
    Math **data;
    Math  *top;   // Pointer to the next free element of the stack.
    int    size;
    int    max;   // Maximum stack size.
} Stack;


Stack* newStack(int maxSize) {
    Stack *stack = (Stack *) malloc(sizeof(Stack));
    stack->data = (Math **) malloc(sizeof(Math *));

    int x;
    for (x = 0; x < maxSize; x++)
        stack->data[x] = (Math *) malloc(sizeof(Math));

    stack->top = stack->data[0];
    stack->max = maxSize;
    stack->size = 0;

    return stack;
}


void freeStack(Stack *stack) {
    int x;
    for (x = 0; x < stack->size; x++)
        free(stack->data[x]);

    free(stack->data);
    free(stack);
}


static int isFull(Stack *stack) {
    if (stack->top - 1 == stack->data[stack->max - 1])
        return 1;
    return 0;
}


static int isEmpty(Stack *stack) {
    if (stack->top == stack->data[0])
        return 1;
    return 0;
}


void push(Stack *stack, Math data) {
    if (isFull(stack))
        return;

    *(stack->top++) = data;
    stack->size++;
}


Math* pop(Stack *stack) {
    if (isEmpty(stack))
        return NULL;

    Math *element = --stack->top;
    stack->size--;

    return element;
}


Math* peek(Stack *stack) {
    return stack->top - 1;
}


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
        if (stack->data[x]->type == SYMBOL)
            printf("%c\n", (char) stack->data[x]->value);
        else
            printf("%d\n", stack->data[x]->value);
    }
    printf("-----Stack Bottom-----\n\n");
}


/**
 * Returns a postfix conversion of infix expression 'e'.
 * URL: https://en.wikipedia.org/wiki/Shunting_yard_algorithm
 * URL: https://raj457036.github.io/Simple-Tools/prefixAndPostfixConvertor.html
 * 
 * The recursive descent parser ensured balanced parentheses.
 */
Expression* shunt(Expression *e) {
    Stack *operator = newStack(e->size);
    Expression *out = newExpression();
    Math *o1 = getNextMath(e);
    
    while (o1 != NULL) {
        // If a number:
        if (o1->type == INTEGER)
            addMath(out, o1->type, o1->assoc, o1->priority, o1->value);

        // If a left parenthesis:
        else if (o1->type == SYMBOL && (char) o1->value == '(')
            push(operator, *o1);
        
        // If a right parenthesis:
        else if (o1->type == SYMBOL && (char) o1->value == ')') {
            while ((char) peek(operator)->value != '(') {
                Math *m = pop(operator);
                addMath(out, m->type, m->assoc, m->priority, m->value);
            }
            pop(operator);  // Discard "(".
        }

        // If an operator:
        else if (o1->type = SYMBOL) {
            while (!isEmpty(operator)) {
                Math *o2 = peek(operator);

                if ((char) o2->value != '(' &&
                    (o2->priority < o1->priority ||
                        (o1->priority == o2->priority && o1->assoc == LEFT)))
                {
                    Math *m = pop(operator);
                    addMath(out, m->type, m->assoc, m->priority, m->value);
                }
                else
                    break;
            }
            push(operator, *o1);
        }
        o1 = getNextMath(e);
    }

    while (!isEmpty(operator)) {
        Math *m = pop(operator);
        addMath(out, m->type, m->assoc, m->priority, m->value);
    }

    freeStack(operator);
    freeExpression(e);
    return out;
}


/**
 * Evaluates postfix expression 'e'.  
 */
int eval(Expression *e) {
    Stack *working = newStack(e->size);
    Math *next = getNextMath(e);

    while (next != NULL) {
        int result;

        // Addition.
        if (next->type == SYMBOL && (char) next->value == '+') {
            result = pop(working)->value + pop(working)->value;
            push(working, (Math) {INTEGER, LEFT, 0, result});
        }

        // Subtraction.
        else if (next->type == SYMBOL &&
                 next->priority == 4 &&
                 (char) next->value == '-') 
        {
            result = (pop(working)->value - pop(working)->value);
            push(working, (Math) {INTEGER, LEFT, 0, -result});  // Negate because stack.
        }

        // Unary minus.
        else if (next->type == SYMBOL &&
                 next->priority == 2 &&
                 (char) next->value == '-') 
        {
            result = pop(working)->value;
            push(working, (Math) {INTEGER, LEFT, 0, -result});
        }

        // Multiplication.
        else if (next->type == SYMBOL && (char) next->value == '*') {
            result = pop(working)->value * pop(working)->value;
            push(working, (Math) {INTEGER, LEFT, 0, result});
        }

        // Integer division.
        else if (next->type == SYMBOL && (char) next->value == '/') {
            int pop1 = pop(working)->value;
            int pop2 = pop(working)->value;
            if (pop1 == 0) {
                printf("Error: Divided by zero.\n");
                //freeStack(working);
                return 0;
            }
            push(working, (Math) {INTEGER, LEFT, 0, pop2 / pop1});
        }

        // Integer.
        else if (next->type == INTEGER)
            push(working, *next);

        next = getNextMath(e);
    }

    int eval = pop(working)->value;
    //freeStack(working); 
    return eval;
}
/** EXPRESSION HANDLING */


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
        addMath(e, SYMBOL, LEFT, 4, t->lexeme[0]);
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
        addMath(e, SYMBOL, LEFT, 3, t->lexeme[0]);
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
        addMath(e, SYMBOL, LEFT, 1, '(');
        
        getNextToken();
        info = parseExpr(e);
        if (info.error != NONE)
            return info;

        checkFor(PAREN_CLOSE, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, SYMBOL, LEFT, 1, ')');
    }
    else if (strcmp(peekNextToken()->lexeme, "-") == 0) {
        addMath(e, SYMBOL, RIGHT, 2, '-');  // Unary minus.
        
        getNextToken();
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, INTEGER, LEFT, 0, atoi(info.token->lexeme));
    }
    else {
        checkFor(OPERAND, &info, CONSUME);
        if (info.error == NONE)
            addMath(e, INTEGER, LEFT, 0, atoi(info.token->lexeme));
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

    Expression *postfix = shunt(expr);  // Infix -> Postfix (RPN)
    //printExpr(postfix);
    printf("= %d\n", eval(postfix));
    freeExpression(postfix);
    stopLexer();
}
