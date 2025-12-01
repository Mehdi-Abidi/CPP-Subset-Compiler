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
        exit(1);
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

// ------------------ Program ------------------
void Parser::program()
{
    semantic.enterScope(); // global scope
    while (current.type != T_EOF)
    {
        if (isType(current.type))
        {
            // Check if it's a function declaration/definition
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
                                exit(1);
                            }
                            string pName = current.value;
                            paramTypes.push_back(pTypeTok == T_INT ? "int" : pTypeTok == T_CHAR ? "char" : pTypeTok == T_BOOL ? "bool" : "void");
                            paramNames.push_back(pName);
                            // Insert parameter into function scope (will be done when entering scope)
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
                    match(T_RPAREN);

                    string retType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void");

                    bool isDefinition = false;
                    if (current.type == T_SEMICOLON)
                    {
                        isDefinition = false;
                        semantic.declareFunction(name, retType, paramTypes, paramNames, nameLine, false);
                        advance();
                    }
                    else if (current.type == T_LBRACE)
                    {
                        isDefinition = true;
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
                        cout << "Syntax Error: expected ';' or '{' after function signature\n";
                        exit(1);
                    }
                }
                else
                {
                    // Variable declaration (type already consumed, name is 'name', and current is next token)
                    bool isArray = false;
                    int arrSize = -1;
                    if (current.type == T_LBRACKET)
                    {
                        advance(); // skip '['
                        if (current.type == T_NUMBER)
                        {
                            arrSize = stoi(current.value);
                            advance(); // skip size
                            match(T_RBRACKET);
                            isArray = true;
                        }
                        else
                        {
                            cout << "Syntax Error: expected array size\n";
                            exit(1);
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
                            cout << "Syntax Error: expected identifier in declaration list\n";
                            exit(1);
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
                                match(T_RBRACKET);
                                vIsArray = true;
                            }
                            else
                            {
                                cout << "Syntax Error: expected array size\n";
                                exit(1);
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

                    match(T_SEMICOLON);
                }
            }
            else
            {
                cout << "Syntax Error: expected identifier after type\n";
                exit(1);
            }
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
            // parameters are handled when entering function body; no direct declare() here anymore
            
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
    match(T_LBRACE);
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
    match(T_RBRACE);
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
                    match(T_RBRACKET);
                    isArray = true;
                    arrSize = size;
                }
                else
                {
                    cout << "Syntax Error: expected array size number\n";
                    exit(1);
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
            match(T_RBRACKET);
            
            if (current.type == T_ASSIGN)
            {
                advance();
                string rhsType = expression();
                semantic.noteInitialization(varName, nameLine);
                match(T_SEMICOLON);
            }
            else
            {
                cout << "Syntax Error: expected '=' after array access\n";
                exit(1);
            }
        }
        else if (current.type == T_LPAREN)
        {
            // Function call
            advance();
            argumentList();
            match(T_RPAREN);
            match(T_SEMICOLON);
            semantic.logFunctionCall(varName, nameLine);
        }
        else if (current.type == T_ASSIGN)
        {
            // Assignment
            advance();
            string rhsType = expression();
            semantic.noteInitialization(varName, nameLine);
            match(T_SEMICOLON);
            semantic.logAssignment(varName, "expression", nameLine);
        }
        else
        {
            cout << "Syntax Error: expected '=', '[', or '('\n";
            exit(1);
        }
    }
    else if (current.type == T_SEMICOLON)
    {
        // Empty statement
        advance();
    }
    else
    {
        cout << "Syntax Error: unexpected token in statement: " << current.value << "\n";
        exit(1);
    }
}

// ------------------ If Statement ------------------
void Parser::ifStatement()
{
    match(T_IF);
    match(T_LPAREN);
    expression(); // condition
    match(T_RPAREN);
    
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
    match(T_FOR);
    match(T_LPAREN);
    
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
            match(T_SEMICOLON);
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
    match(T_SEMICOLON);
    
    // Increment (optional) - can be assignment or expression
    if (current.type != T_RPAREN)
    {
        // Check if it's an assignment (identifier followed by =)
        // We need to peek ahead without consuming
        if (current.type == T_IDENTIFIER)
        {
            string idName = current.value;
            advance(); // consume identifier
            if (current.type == T_ASSIGN)
            {
                // Assignment expression: identifier = expression
                advance(); // consume =
                expression(); // parse right side (should start with identifier, number, etc.)
            }
            else
            {
                // Not an assignment - parse as expression starting from identifier
                // But we already consumed it, so we need to parse from where we are
                // Actually, if it's just an identifier, that's a valid expression
                // If there are operators after, parse them
                if (current.type == T_PLUS || current.type == T_MINUS || 
                    current.type == T_MUL || current.type == T_DIV ||
                    current.type == T_LT || current.type == T_LE || 
                    current.type == T_GT || current.type == T_GE ||
                    current.type == T_EQ || current.type == T_NE ||
                    current.type == T_AND || current.type == T_OR)
                {
                    // Continue parsing the expression from the operator
                    parseTerm(); // Start from term level since we already have the identifier
                }
                // Otherwise it's just an identifier, which is a valid expression
            }
        }
        else
        {
            expression();
        }
    }
    match(T_RPAREN);
    
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
    match(T_RETURN);
    string retType;
    if (current.type != T_SEMICOLON)
    {
        retType = expression();
    }
    match(T_SEMICOLON);
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
            match(T_RBRACKET);
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
            match(T_RPAREN);
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
        match(T_RPAREN);
        return t;
    }
    else
    {
        cout << "Syntax Error: expected number, identifier, character literal, or '(' at "
             << fileName << ":" << current.line << "\n";
        exit(1);
    }
}

// ------------------ Parse Entry ------------------
void Parser::parse()
{
    program();
    semantic.finalize();
}
