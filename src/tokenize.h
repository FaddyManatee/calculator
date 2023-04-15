// Legal symbol set for the calculator.
const char SYMBOLS[6];

typedef enum { INTEGER, SYMBOL, EOS, ERROR } TokenType;
typedef enum { NO_ERROR = -1, ILLEGAL_SYMBOL } LexError;

typedef struct Token {
    TokenType     type;
    LexError      code;
    char         *lexeme;
    struct Token *next;
} Token;

int initLexer(char *input);
Token* getNextToken();
Token* peekNextToken();
