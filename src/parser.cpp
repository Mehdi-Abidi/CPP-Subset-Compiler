#include "parser.h"
#include <iostream>
#include <cstdlib>
using namespace std;

Parser::Parser(Scanner sc) : scanner(sc)
{
    current = scanner.getNextToken();
}

void Parser::advance()
{
    current = scanner.getNextToken();
}

void Parser::match(TokenType t)
{
    if (current.type == t)
        advance();
    else
    {
        cout << "Syntax Error! Expected different token." << endl;
        exit(1);
    }
}

// ------------------ Program ------------------
void Parser::program()
{
    while (current.type != T_EOF)
    {
        if (current.type == T_INT || current.type == T_CHAR || current.type == T_BOOL ||
            current.type == T_DOUBLE || current.type == T_FLOAT || current.type == T_STRING)
        {
            declaration();
        }
        else
        {
            statement();
        }
    }
}

// ------------------ Declarations ------------------
void Parser::declaration()
{
    TokenType type = current.type;
    advance(); // skip type

    while (true)
    {
        if (current.type == T_IDENTIFIER)
        {
            string varName = current.value;
            advance(); // skip identifier

            // Check for array
            if (current.type == T_LBRACKET)
            {
                advance();
                if (current.type == T_NUMBER)
                {
                    int size = stoi(current.value);
                    advance();
                    if (current.type == T_RBRACKET)
                        advance();
                    else
                    {
                        cout << "Syntax Error: expected ']'\n";
                        exit(1);
                    }
                }
                else
                {
                    cout << "Syntax Error: expected array size number\n";
                    exit(1);
                }
            }

            // Optional assignment
            if (current.type == T_ASSIGN)
            {
                advance();
                expression();
            }

            cout << "Declaration parsed: " << varName << "\n";

            if (current.type == T_COMMA)
                advance(); // next variable
            else if (current.type == T_SEMICOLON)
            {
                advance();
                break;
            }
            else
            {
                cout << "Syntax Error: expected ',' or ';'\n";
                exit(1);
            }
        }
        else
        {
            cout << "Syntax Error: expected identifier!\n";
            exit(1);
        }
    }
}

// ------------------ Statements ------------------
void Parser::statement()
{
    if (current.type == T_IDENTIFIER)
    {
        string varName = current.value;
        advance();
        if (current.type == T_ASSIGN)
        {
            advance();
            expression();
            if (current.type == T_SEMICOLON)
                advance();
            else
            {
                cout << "Syntax Error: expected ';'\n";
                exit(1);
            }
        }
        else
        {
            cout << "Syntax Error: expected '='\n";
            exit(1);
        }
    }
    else if (current.type == T_IF)
    {
        advance(); // skip 'if'
        if (current.type != T_LPAREN)
        {
            cout << "Syntax Error: expected '(' after if\n";
            exit(1);
        }
        advance();    // skip '('
        expression(); // condition
        if (current.type != T_RPAREN)
        {
            cout << "Syntax Error: expected ')' after condition\n";
            exit(1);
        }
        advance(); // skip ')'

        if (current.type != T_LBRACE)
        {
            cout << "Syntax Error: expected '{' after if\n";
            exit(1);
        }
        advance(); // skip '{'

        while (current.type != T_RBRACE)
            statement(); // parse inside block
        advance();       // skip '}'
    }
    else
    {
        cout << "Syntax Error: expected statement!\n";
        exit(1);
    }
}

// ------------------ Expressions ------------------
// Recursive descent parser with operator precedence

void Parser::expression()
{
    parseRelational();
    parseLogical();
}
// Logical operators: &&, ||
void Parser::parseLogical()
{
    parseRelational();
    while (current.type == T_AND || current.type == T_OR)
    {
        advance();
        parseRelational();
    }
}

// Relational operators: < <= > >= == !=
void Parser::parseRelational()
{
    parseTerm();
    while (current.type == T_LT || current.type == T_LE || current.type == T_GT || current.type == T_GE || current.type == T_EQ || current.type == T_NE)
    {
        advance();
        parseTerm();
    }
}

// Term: addition/subtraction
void Parser::parseTerm()
{
    parseFactor();
    while (current.type == T_PLUS || current.type == T_MINUS)
    {
        advance();
        parseFactor();
    }
}

// Factor: multiplication/division
void Parser::parseFactor()
{
    parsePrimary();
    while (current.type == T_MUL || current.type == T_DIV)
    {
        advance();
        parsePrimary();
    }
}

// Primary: numbers, identifiers, parentheses
void Parser::parsePrimary()
{
    if (current.type == T_NOT)
    {
        advance();
        parsePrimary(); // applies NOT to the following primary
    }

    if (current.type == T_NUMBER || current.type == T_IDENTIFIER)
    {
        advance();
    }
    else if (current.type == T_LPAREN)
    {
        advance();
        expression();
        if (current.type != T_RPAREN)
        {
            cout << "Syntax Error: expected ')'\n";
            exit(1);
        }
        advance();
    }
    else
    {
        cout << "Syntax Error: expected number, identifier, or '('\n";
        exit(1);
    }
}

// ------------------ Parse Entry ------------------
void Parser::parse()
{
    program();
    cout << "Parsing Completed Successfully!" << endl;
}
