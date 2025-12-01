#include "parser.h"
#include <iostream>
#include <cstdlib>
using namespace std;

Parser::Parser(Scanner sc, const string& fileName)
    : scanner(sc), semantic(fileName), fileName(fileName)
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
        cout << "Syntax Error! Expected " << (int)t << " but got " << (int)current.type << " (" << current.value << ")" << " at " << fileName << ":" << current.line << endl;
        // Don't exit; continue parsing (error recovery)
    }
}

bool Parser::check(TokenType t)
{
    return current.type == t;
}

bool Parser::isType(TokenType t)
{
    return t == T_INT || t == T_CHAR || t == T_BOOL || t == T_VOID;
}

// Helper: skip to next reasonable synchronization point
void Parser::skipToRecovery()
{
    while (current.type != T_SEMICOLON && current.type != T_LBRACE && 
           current.type != T_RBRACE && current.type != T_EOF && 
           !isType(current.type))
    {
        advance();
    }
}

// ------------------ Program ------------------
void Parser::program()
{
    semantic.enterScope(); // global scope
    while (current.type != T_EOF)
    {
        if (isType(current.type))
        {
            // Could be variable declaration or function declaration/definition
            TokenType type = current.type;
            int typeLine = current.line;
            advance();
            
            if (current.type == T_IDENTIFIER || current.type == T_MAIN)
            {
                string name = (current.type == T_MAIN ? "main" : current.value);
                int nameLine = current.line;
                advance();
                
                if (current.type == T_LPAREN)
                {
                    // Function declaration or definition
                    advance(); // skip '('
                    // Parse parameters and collect types/names
                    vector<string> paramTypes;
                    vector<string> paramNames;
                    if (isType(current.type))
                    {
                        while (true)
                        {
                            TokenType pTypeTok = current.type;
                            int pLine = current.line;
                            advance();
                            if (current.type != T_IDENTIFIER)
                            {
                                cout << "Syntax Error: expected parameter name at " << fileName << ":" << current.line << "\n";
                                skipToRecovery();
                                break;
                            }
                            string pName = current.value;
                            paramTypes.push_back(pTypeTok == T_INT ? "int" : pTypeTok == T_CHAR ? "char" : pTypeTok == T_BOOL ? "bool" : "void");
                            paramNames.push_back(pName);
                            advance();
                            if (current.type == T_COMMA)
                            {
                                advance();
                                if (!isType(current.type)) break;
                                continue;
                            }
                            break;
                        }
                    }
                    if (current.type == T_RPAREN) advance();
                    else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";

                    string retType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void");

                    if (current.type == T_SEMICOLON)
                    {
                        // Function declaration
                        semantic.declareFunction(name, retType, paramTypes, paramNames, nameLine, false);
                        advance();
                    }
                    else if (current.type == T_LBRACE)
                    {
                        // Function definition
                        if (semantic.declareFunction(name, retType, paramTypes, paramNames, nameLine, true))
                        {
                            semantic.enterFunctionBody(name);
                            // insert parameters into current function scope
                            for (size_t i = 0; i < paramNames.size(); ++i)
                            {
                                semantic.declareVariable(paramNames[i], paramTypes[i], false, -1, nameLine, "parameter");
                                semantic.noteInitialization(paramNames[i], nameLine);
                            }
                            block();
                            semantic.leaveFunctionBody();
                        }
                        else
                        {
                            // Skip body but still consume it
                            block();
                        }
                    }
                    else
                    {
                        cout << "Syntax Error: expected ';' or '{' after function signature at " << fileName << ":" << current.line << "\n";
                        skipToRecovery();
                        if (current.type == T_SEMICOLON) advance();
                        else if (current.type == T_LBRACE) block();
                    }
                }
                else if (current.type == T_LBRACKET || current.type == T_ASSIGN || 
                         current.type == T_COMMA || current.type == T_SEMICOLON)
                {
                    // Variable declaration
                    bool isArray = false;
                    int arrSize = -1;
                    if (current.type == T_LBRACKET)
                    {
                        advance(); // skip '['
                        if (current.type == T_NUMBER)
                        {
                            arrSize = stoi(current.value);
                            advance(); // skip size
                            if (current.type == T_RBRACKET) advance();
                            else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";
                            isArray = true;
                        }
                        else
                        {
                            cout << "Syntax Error: expected array size at " << fileName << ":" << current.line << "\n";
                            skipToRecovery();
                        }
                    }

                    string baseType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void");
                    bool ok = semantic.declareVariable(name, baseType, isArray, arrSize, nameLine, "variable");
                    string initRepr;
                    if (current.type == T_ASSIGN)
                    {
                        advance();
                        initRepr = "initializer";
                        string exprType = expression();
                        if (ok)
                        {
                            semantic.noteInitialization(name, nameLine);
                        }
                    }

                    if (ok)
                    {
                        string typeRepr = isArray ? baseType + "[" + (arrSize >= 0 ? to_string(arrSize) : "") + "]" : baseType;
                        semantic.logDeclarationParsed(name, typeRepr, nameLine, initRepr);
                    }

                    // Additional variables in same declaration
                    while (current.type == T_COMMA)
                    {
                        advance();
                        if (current.type != T_IDENTIFIER)
                        {
                            cout << "Syntax Error: expected identifier in declaration list at " << fileName << ":" << current.line << "\n";
                            skipToRecovery();
                            break;
                        }
                        string varName = current.value;
                        int vLine = current.line;
                        advance();

                        bool vIsArray = false;
                        int vArrSize = -1;
                        if (current.type == T_LBRACKET)
                        {
                            advance();
                            if (current.type == T_NUMBER)
                            {
                                vArrSize = stoi(current.value);
                                advance();
                                if (current.type == T_RBRACKET) advance();
                                else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";
                                vIsArray = true;
                            }
                            else
                            {
                                cout << "Syntax Error: expected array size at " << fileName << ":" << current.line << "\n";
                                skipToRecovery();
                            }
                        }

                        bool okVar = semantic.declareVariable(varName, baseType, vIsArray, vArrSize, vLine, "variable");
                        string vInitRepr;
                        if (current.type == T_ASSIGN)
                        {
                            advance();
                            vInitRepr = "initializer";
                            string exprType = expression();
                            if (okVar)
                            {
                                semantic.noteInitialization(varName, vLine);
                            }
                        }
                        if (okVar)
                        {
                            string typeRepr = vIsArray ? baseType + "[" + (vArrSize >= 0 ? to_string(vArrSize) : "") + "]" : baseType;
                            semantic.logDeclarationParsed(varName, typeRepr, vLine, vInitRepr);
                        }
                    }

                    if (current.type == T_SEMICOLON) advance();
                    else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
                }
                else
                {
                    cout << "Syntax Error: expected '(', '[', '=', ',', or ';' after identifier at " << fileName << ":" << current.line << "\n";
                    skipToRecovery();
                    if (current.type == T_SEMICOLON) advance();
                }
            }
            else
            {
                cout << "Syntax Error: expected identifier after type at " << fileName << ":" << current.line << "\n";
                skipToRecovery();
            }
        }
        else if (current.type == T_SEMICOLON)
        {
            // Empty statement at global scope
            advance();
        }
        else
        {
            statement();
        }
    }
    semantic.leaveScope();
}

// ------------------ Parameter List ------------------
void Parser::parameterList()
{
    if (isType(current.type))
    {
        TokenType paramType = current.type;
        advance();
        if (current.type == T_IDENTIFIER)
        {
            string paramName = current.value;
            advance();
            
            if (current.type == T_COMMA)
            {
                advance();
                parameterList();
            }
        }
    }
    // Empty parameter list is allowed
}

// ------------------ Argument List ------------------
void Parser::argumentList()
{
    if (current.type != T_RPAREN)
    {
        expression();
        while (current.type == T_COMMA)
        {
            advance();
            expression();
        }
    }
}

// ------------------ Block ------------------
void Parser::block()
{
    if (current.type == T_LBRACE) advance();
    else cout << "Syntax Error: expected '{' at " << fileName << ":" << current.line << "\n";
    
    semantic.enterScope();
    while (current.type != T_RBRACE && current.type != T_EOF)
    {
        if (isType(current.type))
        {
            declaration();
        }
        else
        {
            statement();
        }
    }
    
    if (current.type == T_RBRACE) advance();
    else cout << "Syntax Error: expected '}' at " << fileName << ":" << current.line << "\n";
    
    semantic.leaveScope();
}

// ------------------ Declarations ------------------
void Parser::declaration()
{
    TokenType type = current.type;
    int typeLine = current.line;
    advance(); // skip type

    while (true)
    {
        if (current.type == T_IDENTIFIER)
        {
            string varName = current.value;
            int nameLine = current.line;
            advance(); // skip identifier

            // Check for array
            bool isArray = false;
            int arrSize = -1;
            if (current.type == T_LBRACKET)
            {
                advance();
                if (current.type == T_NUMBER)
                {
                    int size = stoi(current.value);
                    advance();
                    if (current.type == T_RBRACKET) advance();
                    else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";
                    isArray = true;
                    arrSize = size;
                }
                else
                {
                    cout << "Syntax Error: expected array size number at " << fileName << ":" << current.line << "\n";
                    skipToRecovery();
                }
            }

            string baseType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void");
            bool ok = semantic.declareVariable(varName, baseType, isArray, arrSize, nameLine, "variable");

            // Optional assignment
            string initRepr;
            if (current.type == T_ASSIGN)
            {
                advance();
                initRepr = "initializer";
                string exprType = expression();
                if (ok)
                {
                    semantic.noteInitialization(varName, nameLine);
                }
            }

            if (ok)
            {
                string typeRepr = isArray ? baseType + "[" + (arrSize >= 0 ? to_string(arrSize) : "") + "]" : baseType;
                semantic.logDeclarationParsed(varName, typeRepr, nameLine, initRepr);
            }

            if (current.type == T_COMMA)
                advance(); // next variable
            else if (current.type == T_SEMICOLON)
            {
                advance();
                break;
            }
            else
            {
                cout << "Syntax Error: expected ',' or ';' at " << fileName << ":" << current.line << "\n";
                skipToRecovery();
                break;
            }
        }
        else
        {
            cout << "Syntax Error: expected identifier at " << fileName << ":" << current.line << "\n";
            skipToRecovery();
            break;
        }
    }
}

// ------------------ Statements ------------------
void Parser::statement()
{
    if (current.type == T_IF)
    {
        ifStatement();
    }
    else if (current.type == T_FOR)
    {
        forStatement();
    }
    else if (current.type == T_RETURN)
    {
        returnStatement();
    }
    else if (current.type == T_LBRACE)
    {
        block();
    }
    else if (current.type == T_IDENTIFIER)
    {
        string varName = current.value;
        int nameLine = current.line;
        advance();
        
        // Could be assignment, array access, or function call
        if (current.type == T_LBRACKET)
        {
            // Array access
            advance();
            string indexType = expression();
            if (current.type == T_RBRACKET) advance();
            else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";
            
            if (current.type == T_ASSIGN)
            {
                advance();
                string rhsType = expression();
                semantic.noteInitialization(varName, nameLine);
                if (current.type == T_SEMICOLON) advance();
                else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
            }
            else
            {
                cout << "Syntax Error: expected '=' after array access at " << fileName << ":" << current.line << "\n";
                skipToRecovery();
            }
        }
        else if (current.type == T_LPAREN)
        {
            // Function call
            advance();
            argumentList();
            if (current.type == T_RPAREN) advance();
            else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";
            
            if (current.type == T_SEMICOLON) advance();
            else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
            
            semantic.logFunctionCall(varName, nameLine);
        }
        else if (current.type == T_ASSIGN)
        {
            // Assignment
            advance();
            string rhsType = expression();
            semantic.noteInitialization(varName, nameLine);
            if (current.type == T_SEMICOLON) advance();
            else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
            
            semantic.logAssignment(varName, "expression", nameLine);
        }
        else
        {
            cout << "Syntax Error: expected '=', '[', or '(' after identifier at " << fileName << ":" << current.line << "\n";
            skipToRecovery();
        }
    }
    else if (current.type == T_SEMICOLON)
    {
        // Empty statement
        advance();
    }
    else
    {
        cout << "Syntax Error: unexpected token in statement at " << fileName << ":" << current.line << "\n";
        skipToRecovery();
    }
}

// ------------------ If Statement ------------------
void Parser::ifStatement()
{
    if (current.type == T_IF) advance();
    else cout << "Syntax Error: expected 'if' at " << fileName << ":" << current.line << "\n";
    
    if (current.type == T_LPAREN) advance();
    else cout << "Syntax Error: expected '(' at " << fileName << ":" << current.line << "\n";
    
    expression(); // condition
    
    if (current.type == T_RPAREN) advance();
    else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";
    
    if (current.type == T_LBRACE)
    {
        block();
    }
    else
    {
        statement(); // single statement without braces
    }
    
    if (current.type == T_ELSE)
    {
        advance(); // skip 'else'
        if (current.type == T_LBRACE)
        {
            block();
        }
        else
        {
            statement(); // single statement without braces
        }
    }
    semantic.logIfStatement(current.line);
}

// ------------------ For Statement ------------------
void Parser::forStatement()
{
    if (current.type == T_FOR) advance();
    else cout << "Syntax Error: expected 'for' at " << fileName << ":" << current.line << "\n";
    
    if (current.type == T_LPAREN) advance();
    else cout << "Syntax Error: expected '(' at " << fileName << ":" << current.line << "\n";
    
    // Initialization (optional)
    if (current.type != T_SEMICOLON)
    {
        if (isType(current.type))
        {
            declaration(); // declaration with semicolon
        }
        else
        {
            expression();
            if (current.type == T_SEMICOLON) advance();
            else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
        }
    }
    else
    {
        advance(); // skip semicolon
    }
    
    // Condition (optional)
    if (current.type != T_SEMICOLON)
    {
        expression();
    }
    if (current.type == T_SEMICOLON) advance();
    else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
    
    // Increment (optional)
    if (current.type != T_RPAREN)
    {
        if (current.type == T_IDENTIFIER)
        {
            string idName = current.value;
            advance(); // consume identifier
            if (current.type == T_ASSIGN)
            {
                advance(); // consume =
                expression();
            }
            else if (current.type == T_PLUS || current.type == T_MINUS || 
                    current.type == T_MUL || current.type == T_DIV ||
                    current.type == T_LT || current.type == T_LE || 
                    current.type == T_GT || current.type == T_GE ||
                    current.type == T_EQ || current.type == T_NE ||
                    current.type == T_AND || current.type == T_OR)
            {
                parseTerm();
            }
        }
        else
        {
            expression();
        }
    }
    
    if (current.type == T_RPAREN) advance();
    else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";
    
    // Body
    if (current.type == T_LBRACE)
    {
        block();
    }
    else
    {
        statement();
    }
    semantic.logForLoop(current.line);
}

// ------------------ Return Statement ------------------
void Parser::returnStatement()
{
    int retLine = current.line;
    if (current.type == T_RETURN) advance();
    else cout << "Syntax Error: expected 'return' at " << fileName << ":" << current.line << "\n";
    
    string retType;
    if (current.type != T_SEMICOLON)
    {
        retType = expression();
    }
    
    if (current.type == T_SEMICOLON) advance();
    else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
    
    semantic.noteReturn(retLine, retType);
    semantic.logReturn(retLine);
}

// ------------------ Expressions ------------------
// Recursive descent parser with operator precedence
// Precedence (lowest to highest): logical (&&, ||) -> relational (<, <=, >, >=, ==, !=) -> term (+, -) -> factor (*, /) -> primary
string Parser::expression()
{
    return parseLogical();
}

// Logical operators: &&, || (lowest precedence)
string Parser::parseLogical()
{
    string leftType = parseRelational();
    while (current.type == T_AND || current.type == T_OR)
    {
        Token op = current;
        advance();
        string rightType = parseRelational();
        if (leftType != "bool" || rightType != "bool")
        {
            semantic.logError("Logical operator requires boolean operands", op.line);
        }
        leftType = "bool";
    }
    return leftType;
}

// Relational operators: < <= > >= == !=
string Parser::parseRelational()
{
    string leftType = parseTerm();
    while (current.type == T_LT || current.type == T_LE || current.type == T_GT || 
           current.type == T_GE || current.type == T_EQ || current.type == T_NE)
    {
        Token op = current;
        advance();
        string rightType = parseTerm();
        if (leftType != rightType)
        {
            semantic.logError("Type mismatch in relational expression", op.line);
        }
        leftType = "bool";
    }
    return leftType;
}

// Term: addition/subtraction
string Parser::parseTerm()
{
    string leftType = parseFactor();
    while (current.type == T_PLUS || current.type == T_MINUS)
    {
        Token op = current;
        advance();
        string rightType = parseFactor();
        if (!(leftType == "int" && rightType == "int"))
        {
            semantic.logError("Arithmetic operator requires integer operands", op.line);
        }
        leftType = "int";
    }
    return leftType;
}

// Factor: multiplication/division
string Parser::parseFactor()
{
    string leftType = parsePrimary();
    while (current.type == T_MUL || current.type == T_DIV)
    {
        Token op = current;
        advance();
        string rightType = parsePrimary();
        if (!(leftType == "int" && rightType == "int"))
        {
            semantic.logError("Arithmetic operator requires integer operands", op.line);
        }
        leftType = "int";
    }
    return leftType;
}

// Primary: numbers, identifiers, parentheses, array access, function calls
string Parser::parsePrimary()
{
    // Handle unary NOT operator
    if (current.type == T_NOT)
    {
        Token op = current;
        advance();
        string innerType = parsePrimary();
        if (innerType != "bool")
        {
            semantic.logError("Logical not '!' requires boolean operand", op.line);
        }
        return "bool";
    }

    if (current.type == T_NUMBER)
    {
        string t = "int";
        advance();
        return t;
    }
    if (current.type == T_CHAR_LITERAL)
    {
        string t = "char";
        advance();
        return t;
    }
    else if (current.type == T_IDENTIFIER)
    {
        string name = current.value;
        int nameLine = current.line;
        advance();
        
        // Check for array access or function call
        if (current.type == T_LBRACKET)
        {
            // Array access
            advance();
            expression();
            if (current.type == T_RBRACKET) advance();
            else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";
            
            semantic.noteUse(name, nameLine, "array access");
            return semantic.getVariableType(name, nameLine);
        }
        else if (current.type == T_LPAREN)
        {
            // Function call
            advance();
            vector<string> argTypes;
            if (current.type != T_RPAREN)
            {
                argTypes.push_back(expression());
                while (current.type == T_COMMA)
                {
                    advance();
                    argTypes.push_back(expression());
                }
            }
            if (current.type == T_RPAREN) advance();
            else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";
            
            semantic.logFunctionCall(name, nameLine);
            // For now assume functions return int by default
            return "int";
        }
        else
        {
            semantic.noteUse(name, nameLine, "expression");
            return semantic.getVariableType(name, nameLine);
        }
    }
    else if (current.type == T_LPAREN)
    {
        advance();
        string t = expression();
        if (current.type == T_RPAREN) advance();
        else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";
        return t;
    }
    else
    {
        cout << "Syntax Error: expected number, identifier, character literal, or '(' at "
             << fileName << ":" << current.line << "\n";
        skipToRecovery();
        return "int";
    }
}

// ------------------ Parse Entry ------------------
void Parser::parse()
{
    program();
    semantic.finalize();
}
