#pragma once
#include "scanner.h"
#include "semantic.h"
using namespace std;
class Parser
{
private:
    Scanner scanner;// Lexical scanner
    Token current;// Current token being processed
    Semantic semantic;// Semantic analyzer
    string fileName;// Source file name

    void advance();// Advance to the next token
    void match(TokenType t);// Match and consume a token of expected type
    bool check(TokenType t);// Check if current token matches expected type
    bool isType(TokenType t);// Check if token is a type specifier
    void skipToRecovery();// Error recovery: skip tokens until a synchronization point

    void program();// Entry point for parsing
    void declaration();// Variable or function declaration
    void statement();// Statement parsing
    void block();// Block of statements
    string expression();// Expression parsing

    // Function parsing
    void functionDeclaration();// Function declaration parsing
    void functionDefinition();// Function definition parsing
    void functionCall();// Function call parsing
    TokenType parseType();// Parse a type specifier
    void parameterList();// Parse function parameter list
    void argumentList();// Parse function argument list

    // Control flow
    void ifStatement();// If statement parsing
    void forStatement();// For loop parsing
    void returnStatement();// Return statement parsing

    // Recursive descent helpers for operator precedence
    string parseLogical();   // handles &&, || (lowest precedence)
    string parseRelational(); // handles <, <=, >, >=, ==, !=
    string parseTerm();      // handles +, -
    string parseFactor();    // handles *, /
    string parsePrimary();   // handles numbers, identifiers, parentheses, array access, function calls

public:
    Parser(Scanner sc, const string& fileName);// Constructor
    void parse();// Start parsing process
};
