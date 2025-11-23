#pragma once
#include "scanner.h"
#include "semantic.h"

class Parser {
private:
    Scanner scanner;
    Token current;

    void advance();
    void match(TokenType t);

    void program();
    void declaration();
    void statement();
    void expression();

public:
    Parser(Scanner sc);
    void parse();
};
