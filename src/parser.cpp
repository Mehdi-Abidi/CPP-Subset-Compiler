#include "parser.h"
#include <iostream>
using namespace std;

Parser::Parser(Scanner sc) : scanner(sc) {
    current = scanner.getNextToken();
}

void Parser::advance() {
    current = scanner.getNextToken();
}

void Parser::match(TokenType t) {
    if (current.type == t) advance();
    else {
        cout << "Syntax Error!" << endl;
        exit(1);
    }
}

void Parser::program() {
    while(current.type != T_EOF) {
        declaration();
    }
}

void Parser::declaration() {
    // Only handles int, char, bool
    if(current.type == T_INT || current.type == T_CHAR || current.type == T_BOOL
       || current.type == T_FLOAT || current.type == T_DOUBLE
       || current.type == T_STRING) {
        TokenType type = current.type;
        advance(); // skip type

        if(current.type == T_IDENTIFIER) {
            advance(); // skip variable name
            if(current.type == T_SEMICOLON) {
                advance(); // skip semicolon
                cout << "Declaration parsed.\n";
            } else {
                cout << "Syntax Error: missing semicolon!\n";
                exit(1);
            }
        } else {
            cout << "Syntax Error: expected identifier!\n";
            exit(1);
        }
    } else {
        cout << "Syntax Error: expected type!\n";
        exit(1);
    }
}


void Parser::statement() {
    // You can expand later
}

void Parser::expression() {
    // You can expand later
}

void Parser::parse() {
    program();
    cout << "Parsing Completed Successfully!" << endl;
}
