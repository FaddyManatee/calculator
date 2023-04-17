#include "tokenize.h"

// Syntax errors which may be encountered by the parser.
typedef enum {
    NONE,
    OPERATOR,    // Expected an operator next to a parenthesis.
    OPERAND,     // Expected an integer.
    PAREN_CLOSE  // ) expected.
} SyntaxError;

typedef struct {
    Token       *token;
    SyntaxError  error;
} ParserInfo;

void parse(char *input);  // Initialise the parser to parse CLI input.
