#include "tokenize.h"

// Syntax errors which my be encountered by the parser.
typedef enum {
    NONE,
    OPERATOR,    // Expected a '+', '-', '*' or '/'.
    OPERAND,     // Expected an integer.
    PAREN_CLOSE  // ) expected.
} SyntaxError;

typedef struct {
    Token       *token;
    SyntaxError  error;
} ParserInfo;

int InitParser(char *input);  // Initialise the parser to parse CLI input.
int StopParser();             // Stop the parser and do any necessary clean up.
