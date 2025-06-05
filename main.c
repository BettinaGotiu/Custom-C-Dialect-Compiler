#include <stdio.h>
#include "utils.h"
#include "stdlib.h"
#include "lexer.h"
#include "parser.h"
#include "ad.h"


int main() {
    char *inbuf=loadFile("tests/testad.c");
    /*Token *tokens = tokenize(inbuf);
    parse(tokens);*/
    Token* parselist = tokenize(inbuf);
    showTokens(parselist);
    pushDomain();
    parse(parselist);
    showDomain(symTable,"global");
    dropDomain();
    return 0;
}
