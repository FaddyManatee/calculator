#include <stdio.h>
#include "calcparser.h"


int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Usage: ./calculator [expression]\n");
        return 0;
    }
    else if (argc > 2) {
        printf("Error: expected only one argument.\n");
        return 0;        
    }

    parse(argv[1]);
    return 0;
}
