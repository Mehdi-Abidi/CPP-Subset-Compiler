#pragma once
#include "scanner.h"
#include "semantic.h"

class Parser
{
private:
    Scanner scanner;
    Token current;
    Semantic semantic;
    string fileName;

    void advance();
    void match(TokenType t);
    bool check(TokenType t);
    bool isType(TokenType t);
    void skipToRecovery();

    void program();
    void declaration();
    void statement();
    void block();
    string expression();

    // Function parsing
    void functionDeclaration();
    void functionDefinition();
    void functionCall();
    TokenType parseType();
    void parameterList();
    void argumentList();

    // Control flow
    void ifStatement();
    void forStatement();
    void returnStatement();

    // Recursive descent helpers for operator precedence
    string parseLogical();   // handles &&, || (lowest precedence)
    string parseRelational(); // handles <, <=, >, >=, ==, !=
    string parseTerm();      // handles +, -
    string parseFactor();    // handles *, /
    string parsePrimary();   // handles numbers, identifiers, parentheses, array access, function calls

public:
    Parser(Scanner sc, const string& fileName);
    void parse();
};
