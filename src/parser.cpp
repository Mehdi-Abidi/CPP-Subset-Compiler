#include "parser.h" // This includes the header file for the Parser class and related types
#include <iostream> // This lets us use input/output like cout
#include <cstdlib> // This includes general utilities (not strictly needed for all code)
using namespace std; // This lets us write std::cout as cout without the std::

 // [Syntax Analysis with Semantic Hooks] Constructor initializes parser and semantic context
Parser::Parser(Scanner sc, const string& fileName)// Constructor // This is the function that sets up a new Parser
    : scanner(sc), semantic(fileName), fileName(fileName) // This initializes member variables using the given values
{
    current = scanner.getNextToken();// Initialize current token // This asks the scanner for the first token
}

 // [Syntax Analysis] Advance the token stream
void Parser::advance()// Advance to the next token // This function moves to the next token
{
    current = scanner.getNextToken();// Get next token from scanner // Get the next token from the scanner
}

 // [Syntax Analysis] Token matching with simple error reporting
void Parser::match(TokenType t)// Match and consume a token of expected type // This checks if the current token is what we expect
{
    if (current.type == t)// If matches // If the type matches what we expect
        advance();// consume it // Move to the next token
    else // If it does not match
    {
        cout << "Syntax Error! Expected " << (int)t << " but got " << (int)current.type << " (" << current.value << ")" << " at " << fileName << ":" << current.line << endl; // Print an error message showing what we expected, what we got, file name and line number
        // Don't exit; continue parsing (error recovery) // We keep going to try to recover
    }
}

 // [Syntax Analysis] Utility: check current token type
bool Parser::check(TokenType t)// Check if current token matches expected type // This returns true if the current token matches t
{
    return current.type == t;// Return true if matches // Compare types and return result
}

 // [Syntax Analysis] Utility: classify token as a type
bool Parser::isType(TokenType t) // This checks if a token type is a data type
{
    return t == T_INT || t == T_CHAR || t == T_BOOL || t == T_VOID; // Return true if token is one of int, char, bool, void
}

 // [Syntax Analysis] Error recovery to synchronization points
void Parser::skipToRecovery() // This skips tokens until we reach a safe point
{
    while (current.type != T_SEMICOLON && current.type != T_LBRACE &&  // While current token is not a semicolon or {
           current.type != T_RBRACE && current.type != T_EOF &&  // And not a } or end of file
           !isType(current.type)) // And not a type keyword
    {
        advance(); // Move forward one token
    }
}

 // ------------------ Program ------------------
 // [Syntax Analysis with Semantic Hooks] Parse top-level declarations and functions
void Parser::program() // This parses the whole file/program
{
    semantic.enterScope(); // global scope // Tell the semantic system we entered the global scope
    while (current.type != T_EOF)// While not end of file // Keep going until the end
    {
        if (isType(current.type))// Type indicates declaration // If we see a type, it starts a declaration
        {
            // Could be variable declaration or function declaration/definition // It might be a variable or a function
            TokenType type = current.type; // Save the type token
            int typeLine = current.line; // Save the line number of the type
            advance(); // Move past the type
            
            if (current.type == T_IDENTIFIER || current.type == T_MAIN)// Identifier or main function // After type, expect a name or 'main'
            {
                string name = (current.type == T_MAIN ? "main" : current.value);// Get name // If 'main', name is "main"; otherwise token value
                int nameLine = current.line; // Remember the line of the name
                advance(); // Move past the name
                
                if (current.type == T_LPAREN) // If next token is '(' then it is a function
                {
                    // Function declaration or definition // We will parse a function
                    advance(); // skip '(' // Move past '('
                    // Parse parameters and collect types/names // Set up lists for parameter info
                    vector<string> paramTypes;// parameter types // List of parameter types
                    vector<string> paramNames;// parameter names // List of parameter names
                    if (isType(current.type))// has parameters // If there is a type, there are parameters
                    {
                        while (true)// parse parameters // Loop to parse parameters
                        {
                            TokenType pTypeTok = current.type;// parameter type // Save parameter type token
                            int pLine = current.line;// parameter line // Save parameter line number
                            advance(); // Move past parameter type
                            if (current.type != T_IDENTIFIER) // Expect a parameter name
                            {
                                cout << "Syntax Error: expected parameter name at " << fileName << ":" << current.line << "\n"; // error report of missing parameter name // Print error if name missing
                                skipToRecovery(); // Try to recover
                                break; // Stop parsing parameters
                            }
                            string pName = current.value; // Get the parameter name text
                            paramTypes.push_back(pTypeTok == T_INT ? "int" : pTypeTok == T_CHAR ? "char" : pTypeTok == T_BOOL ? "bool" : "void"); // Convert token type to string and add to list
                            paramNames.push_back(pName); // Add parameter name to list
                            advance(); // Move past the name
                            if (current.type == T_COMMA) // If there is a comma, more parameters follow
                            {
                                advance(); // Move past comma
                                if (!isType(current.type)) break;// error will be reported in next loop // If next thing is not a type, stop (error will be elsewhere)
                                continue; // Continue loop for next parameter
                            }
                            break; // No more parameters
                        }
                    }
                    if (current.type == T_RPAREN) advance(); // If we see ')', close the parameter list and move on
                    else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n"; // Otherwise, report missing ')'

                    string retType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void"); // Convert return type token to string

                    if (current.type == T_SEMICOLON)//i there is a semicolon after the function signature then it is a declaration // If there is a ';', it is only a declaration
                    {
                        // Function declaration // We record declaration only
                        semantic.declareFunction(name, retType, paramTypes, paramNames, nameLine, false);//it is a declaration only  // Tell semantic system about the function declaration
                        advance(); // Move past the ';'
                    }
                    else if (current.type == T_LBRACE)//if there is a { after the function signature then it is a definition // If there is a '{', it is a definition with a body
                    {
                        // Function definition // We will parse the body
                        if (semantic.declareFunction(name, retType, paramTypes, paramNames, nameLine, true))//it is a definition // Tell semantic system about the function definition
                        {
                            semantic.enterFunctionBody(name);// enter function scope like its body // Enter function scope for semantics
                            // insert parameters into current function scope // Put parameters into scope
                            for (size_t i = 0; i < paramNames.size(); ++i) // Loop over each parameter
                            {
                                semantic.declareVariable(paramNames[i], paramTypes[i], false, -1, nameLine, "parameter");//it is a parameter // Declare parameter variable in current scope
                                semantic.noteInitialization(paramNames[i], nameLine);// it is initialized upon function entry  // Mark parameter as initialized when function starts
                            }
                            block();// parse function body // Parse the function body block
                            semantic.leaveFunctionBody();// leave function scope // Leave function scope after body
                        }
                        else // If function declaration failed (e.g., duplicate)
                        {
                            // Skip body but still consume it // We still parse the block to move on
                            block(); // Parse the block but ignore semantics
                        }
                    }
                    else // If neither ';' nor '{' follows
                    {
                        cout << "Syntax Error: expected ';' or '{' after function signature at " << fileName << ":" << current.line << "\n"; // Show an error about missing ';' or '{'
                        skipToRecovery(); // Try to recover
                        if (current.type == T_SEMICOLON) advance(); // If we find ';', move past it
                        else if (current.type == T_LBRACE) block(); // If we find '{', parse the block
                    }
                }
                else if (current.type == T_LBRACKET || current.type == T_ASSIGN ||  // If after the name we see [ or = or , or ;
                         current.type == T_COMMA || current.type == T_SEMICOLON)// this condition is checking for variable declaration in global scope  // Then this is a variable declaration
                {
                    // Variable declaration // Handle variable declaration
                    bool isArray = false; // Assume not an array
                    int arrSize = -1; // No size yet
                    if (current.type == T_LBRACKET) // If we see a '[' then it is an array
                    {
                        advance(); // skip '[' // Move past '['
                        if (current.type == T_NUMBER) // Expect a number for size
                        {
                            arrSize = stoi(current.value); // Convert the text to a number and store
                            advance(); // skip size // Move past the number
                            if (current.type == T_RBRACKET) advance(); // Expect and skip ']'
                            else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n"; // Report missing ']'
                            isArray = true; // Mark as array
                        }
                        else // If not a number after '['
                        {
                            cout << "Syntax Error: expected array size at " << fileName << ":" << current.line << "\n"; // Report missing size
                            skipToRecovery(); // Try to recover
                        }
                    }

                    string baseType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void"); // Convert the base type to a string
                    bool ok = semantic.declareVariable(name, baseType, isArray, arrSize, nameLine, "variable"); // Tell semantic system we declared a variable
                    string initRepr; // This will describe the initializer (if present)
                    if (current.type == T_ASSIGN) // If we see '=' then there is an initializer
                    {
                        advance(); // Move past '='
                        initRepr = "initializer"; // Mark that we have an initializer
                        string exprType = expression(); // Parse the expression for the initializer
                        if (ok) // If declaration was valid
                        {
                            semantic.noteInitialization(name, nameLine); // Mark variable as initialized
                        }
                    }

                    if (ok) // If declaration was valid
                    {
                        string typeRepr = isArray ? baseType + "[" + (arrSize >= 0 ? to_string(arrSize) : "") + "]" : baseType; // Build a nice type string (with array size if any)
                        semantic.logDeclarationParsed(name, typeRepr, nameLine, initRepr); // Log the declaration details
                    }

                    // Additional variables in same declaration // Support declarations like: int a, b = 3;
                    while (current.type == T_COMMA) // While we see commas, more variables follow
                    {
                        advance(); // Move past ','
                        if (current.type != T_IDENTIFIER) // Expect a variable name
                        {
                            cout << "Syntax Error: expected identifier in declaration list at " << fileName << ":" << current.line << "\n"; // Report missing name
                            skipToRecovery(); // Try to recover
                            break; // Stop this list
                        }
                        string varName = current.value; // Save the next variable name
                        int vLine = current.line; // Save the line number
                        advance(); // Move past the name

                        bool vIsArray = false; // Assume not an array
                        int vArrSize = -1; // No size yet
                        if (current.type == T_LBRACKET) // If it is an array
                        {
                            advance(); // Skip '['
                            if (current.type == T_NUMBER) // Expect a number
                            {
                                vArrSize = stoi(current.value); // Convert to int
                                advance(); // Move past size
                                if (current.type == T_RBRACKET) advance(); // Expect and skip ']'
                                else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n"; // Report missing ']'
                                vIsArray = true; // Mark as array
                            }
                            else // If there is no number
                            {
                                cout << "Syntax Error: expected array size at " << fileName << ":" << current.line << "\n"; // Report missing size
                                skipToRecovery(); // Try to recover
                            }
                        }

                        bool okVar = semantic.declareVariable(varName, baseType, vIsArray, vArrSize, vLine, "variable"); // Declare the next variable
                        string vInitRepr; // Store initializer description for this variable
                        if (current.type == T_ASSIGN) // If there is an initializer
                        {
                            advance(); // Move past '='
                            vInitRepr = "initializer"; // Note that there is an initializer
                            string exprType = expression(); // Parse the initializer expression
                            if (okVar) // If declaration was valid
                            {
                                semantic.noteInitialization(varName, vLine); // Mark variable as initialized
                            }
                        }
                        if (okVar) // If declaration was valid
                        {
                            string typeRepr = vIsArray ? baseType + "[" + (vArrSize >= 0 ? to_string(vArrSize) : "") + "]" : baseType; // Build type string with array size if needed
                            semantic.logDeclarationParsed(varName, typeRepr, vLine, vInitRepr); // Log declaration info
                        }
                    }

                    if (current.type == T_SEMICOLON) advance(); // Expect ';' at end of declaration and move past it
                    else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n"; // Report missing ';'
                }
                else // If after type and name we did not see function or variable patterns
                {
                    cout << "Syntax Error: expected '(', '[', '=', ',', or ';' after identifier at " << fileName << ":" << current.line << "\n"; // Report unexpected token
                    skipToRecovery(); // Try to recover
                    if (current.type == T_SEMICOLON) advance(); // If we find ';', skip it
                }
            }
            else // If after type we did not find an identifier
            {
                cout << "Syntax Error: expected identifier after type at " << fileName << ":" << current.line << "\n"; // Report missing name
                skipToRecovery(); // Try to recover
            }
        }
        else if (current.type == T_SEMICOLON) // If we see a stray ';' at global scope
        {
            // Empty statement at global scope // This is allowed, just skip it
            advance(); // Move past ';'
        }
        else // If we see something else at global scope
        {
            statement(); // Try to parse it as a statement
        }
    }
    semantic.leaveScope(); // Leave global scope at end of file
}

 // ------------------ Parameter List ------------------
 // [Syntax Analysis] Parse parameter list (types and names)
void Parser::parameterList() // This parses a list of parameters inside function parentheses
{
    if (isType(current.type)) // If we see a type, there is at least one parameter
    {
        TokenType paramType = current.type; // Save the parameter type
        advance(); // Move past the type
        if (current.type == T_IDENTIFIER) // Expect parameter name
        {
            string paramName = current.value; // Save the parameter name
            advance(); // Move past the name
            
            if (current.type == T_COMMA) // If there is a comma
            {
                advance(); // Move past the comma
                parameterList(); // Parse the next parameter
            }
        }
    }
    // Empty parameter list is allowed // If there is no type, parameters can be empty
}

 // ------------------ Argument List ------------------
 // [Syntax Analysis] Parse argument expressions
void Parser::argumentList() // This parses arguments in a function call
{
    if (current.type != T_RPAREN) // If we do not have ')', there is at least one argument
    {
        expression(); // Parse the first argument
        while (current.type == T_COMMA) // While there is a comma, more arguments exist
        {
            advance(); // Move past comma
            expression(); // Parse the next argument
        }
    }
}

 // ------------------ Block ------------------
 // [Syntax Analysis with Semantic Hooks] Parse a block and manage scope
void Parser::block() // This parses a block of code enclosed by { and }
{
    if (current.type == T_LBRACE) advance(); // If we see '{', move past it
    else cout << "Syntax Error: expected '{' at " << fileName << ":" << current.line << "\n"; // Report missing '{'
    
    semantic.enterScope(); // Enter a new scope for variables inside the block
    while (current.type != T_RBRACE && current.type != T_EOF) // While not '}' or end of file
    {
        if (isType(current.type)) // If we see a type, it is a declaration
        {
            declaration(); // Parse the declaration
        }
        else // Otherwise
        {
            statement(); // Parse a statement
        }
    }
    
    if (current.type == T_RBRACE) advance(); // If we see '}', move past it
    else cout << "Syntax Error: expected '}' at " << fileName << ":" << current.line << "\n"; // Report missing '}'
    
    semantic.leaveScope(); // Leave the scope created for this block
}

 // ------------------ Declarations ------------------
 // [Syntax Analysis with Semantic Hooks] Parse variable declarations
void Parser::declaration()
{
    TokenType type = current.type;
    int typeLine = current.line;
    advance();

    while (true)
    {
        if (current.type == T_IDENTIFIER)
        {
            string varName = current.value;
            int nameLine = current.line;
            advance();

            // Check for array // See if variable is an array
            bool isArray = false; // Start with not an array
            int arrSize = -1; // Unknown size
            if (current.type == T_LBRACKET) // If we see '[' then array size should follow
            {
                advance(); // Move past '['
                if (current.type == T_NUMBER) // Expect a number as size
                {
                    int size = stoi(current.value); // Convert text to number
                    advance(); // Move past the number
                    if (current.type == T_RBRACKET) advance(); // Expect and move past ']'
                    else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n"; // Report missing ']'
                    isArray = true; // Mark as array
                    arrSize = size; // Save the size
                }
                else // If not a number after '['
                {
                    cout << "Syntax Error: expected array size number at " << fileName << ":" << current.line << "\n"; // Report missing size
                    skipToRecovery(); // Try to recover
                }
            }

            string baseType = (type == T_INT ? "int" : type == T_CHAR ? "char" : type == T_BOOL ? "bool" : "void"); // Convert base type to string
            bool ok = semantic.declareVariable(varName, baseType, isArray, arrSize, nameLine, "variable"); // Tell semantic system we declared a variable

            // Optional assignment // Handle optional initializer like = 5
            string initRepr; // A string that describes initializer presence
            if (current.type == T_ASSIGN) // If there is an '='
            {
                advance(); // Move past '='
                initRepr = "initializer"; // Set initializer description
                string exprType = expression(); // Parse the initializer expression
                if (ok) // If declaration is valid
                {
                    semantic.checkAssignmentType(varName, exprType, nameLine); // CHECK TYPE HERE TOO
                    semantic.noteInitialization(varName, nameLine); // Mark variable initialized
                }
            }

            if (ok) // If declaration is valid
            {
                string typeRepr = isArray ? baseType + "[" + (arrSize >= 0 ? to_string(arrSize) : "") + "]" : baseType; // Build type string (include size if array)
                semantic.logDeclarationParsed(varName, typeRepr, nameLine, initRepr); // Log declaration info for debugging/report
            }

            if (current.type == T_COMMA) // If there is a comma, more variables follow
                advance(); // next variable // Move past comma
            else if (current.type == T_SEMICOLON) // If we see ';', the declaration ends
            {
                advance(); // Move past ';'
                break; // Stop parsing this declaration
            }
            else // If not comma or semicolon
            {
                cout << "Syntax Error: expected ',' or ';' at " << fileName << ":" << current.line << "\n"; // Report unexpected token
                skipToRecovery(); // Try to recover
                break; // Stop this declaration
            }
        }
        else // If we did not find an identifier
        {
            cout << "Syntax Error: expected identifier at " << fileName << ":" << current.line << "\n"; // Report missing variable name
            skipToRecovery(); // Try to recover
            break; // End the loop
        }
    }
}

 // ------------------ Statements ------------------
 // [Syntax Analysis with Semantic Hooks] Parse statements and trigger semantic notes
void Parser::statement()
{
    if (current.type == T_IF) // If we see 'if'
    {
        ifStatement(); // Parse the if statement
    }
    else if (current.type == T_FOR) // If we see 'for'
    {
        forStatement(); // Parse the for loop
    }
    else if (current.type == T_RETURN) // If we see 'return'
    {
        returnStatement(); // Parse the return statement
    }
    else if (current.type == T_LBRACE) // If we see '{'
    {
        block(); // Parse a block
    }
    else if (current.type == T_IDENTIFIER)
    {
        string varName = current.value;
        int nameLine = current.line;
        advance();
        
        if (current.type == T_LBRACKET)
        {
            advance(); // skip '['
            if (current.type == T_NUMBER) {
                int idx = stoi(current.value);
                semantic.checkArrayIndex(varName, idx, current.line); // bounds check
                advance();
            } else {
                string indexType = expression(); // Get type of index expression
                semantic.checkArrayIndexType(indexType, current.line); // Check it is int
            }
            if (current.type == T_RBRACKET) advance();
            else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";
            
            if (current.type == T_ASSIGN)
            {
                advance(); // skip '='
                string rhsType = expression(); // Get type of RHS value
                semantic.checkArrayElementAssignment(varName, rhsType, nameLine); // CHECK TYPE MATCH
                semantic.noteInitialization(varName, nameLine);
                semantic.logAssignment(varName + "[index]", rhsType, nameLine); // Log the assignment
                
                if (current.type == T_SEMICOLON) advance();
                else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
            }
            else
            {
                cout << "Syntax Error: expected '=' after array access at " << fileName << ":" << current.line << "\n";
                skipToRecovery();
            }
        }
        else if (current.type == T_LPAREN) // Function call statement
        {
            advance(); // skip '('
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
            
            semantic.logFunctionCall(varName, nameLine);
            semantic.checkFunctionCall(varName, argTypes, nameLine); // add this check here too
            
            if (current.type == T_SEMICOLON) advance();
            else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n";
        }
        else if (current.type == T_ASSIGN) // Variable assignment
        {
            advance(); // skip '='
            string rhsType = expression(); // Parse RHS and get its type
            semantic.checkAssignmentType(varName, rhsType, nameLine); // CHECK TYPE MATCH
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
    else if (current.type == T_SEMICOLON) // If we see ';' alone
    {
        // Empty statement // It means do nothing
        advance(); // Move past ';'
    }
    else // If none of the statement types matched
    {
        cout << "Syntax Error: unexpected token in statement at " << fileName << ":" << current.line << "\n"; // Report unexpected token
        skipToRecovery(); // Try to recover
    }
}

 // ------------------ If Statement ------------------
 // [Syntax Analysis with Semantic Hooks] Parse if/else structures
void Parser::ifStatement() // This parses an if statement with optional else
{
    if (current.type == T_IF) advance(); // Move past 'if' keyword
    else cout << "Syntax Error: expected 'if' at " << fileName << ":" << current.line << "\n"; // Report missing 'if'
    
    if (current.type == T_LPAREN) advance(); // Expect '(' after if and move past it
    else cout << "Syntax Error: expected '(' at " << fileName << ":" << current.line << "\n"; // Report missing '('
    
    expression(); // condition // Parse the condition inside the parentheses
    
    if (current.type == T_RPAREN) advance(); // Expect ')' and move past it
    else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n"; // Report missing ')'
    
    if (current.type == T_LBRACE) // If we see '{'
    {
        block(); // Parse the block as the if body
    }
    else // If there is no '{'
    {
        statement(); // single statement without braces // Parse a single statement as the if body
    }
    
    if (current.type == T_ELSE) // If we see 'else'
    {
        advance(); // skip 'else' // Move past else
        if (current.type == T_LBRACE) // If there is a '{'
        {
            block(); // Parse the else block
        }
        else // If no '{'
        {
            statement(); // single statement without braces // Parse a single statement for else
        }
    }
    semantic.logIfStatement(current.line); // Log that we parsed an if statement
}

 // ------------------ For Statement ------------------
 // [Syntax Analysis with Semantic Hooks] Parse for-loops
void Parser::forStatement() // This parses a for loop
{
    if (current.type == T_FOR) advance(); // Move past 'for' keyword
    else cout << "Syntax Error: expected 'for' at " << fileName << ":" << current.line << "\n"; // Report missing 'for'
    
    if (current.type == T_LPAREN) advance(); // Expect '(' and move past it
    else cout << "Syntax Error: expected '(' at " << fileName << ":" << current.line << "\n"; // Report missing '('
    
    // Initialization (optional) // First part inside for parentheses
    if (current.type != T_SEMICOLON) // If not just ';'
    {
        if (isType(current.type)) // If we see a type
        {
            declaration(); // declaration with semicolon // Parse a declaration like int i = 0;
        }
        else // Otherwise
        {
            expression(); // Parse an expression like i = 0
            if (current.type == T_SEMICOLON) advance(); // Expect ';' to end init part
            else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n"; // Report missing ';'
        }
    }
    else // If current is ';'
    {
        advance(); // skip semicolon // Move past ';' to next part
    }
    
    // Condition (optional) // Second part inside for parentheses
    if (current.type != T_SEMICOLON) // If not just ';'
    {
        expression(); // Parse the loop condition like i < 10
    }
    if (current.type == T_SEMICOLON) advance(); // Expect ';' to end condition part
    else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n"; // Report missing ';'
    
    // Increment (optional) // Third part inside for parentheses
    if (current.type != T_RPAREN) // If not ')'
    {
        if (current.type == T_IDENTIFIER) // If it starts with a name
        {
            string idName = current.value; // Save the name
            advance(); // consume identifier // Move past the name
            if (current.type == T_ASSIGN) // If '=' follows
            {
                advance(); // consume = // Move past '='
                expression(); // Parse the expression like i = i + 1
            }
            else if (current.type == T_PLUS || current.type == T_MINUS || // If we see an operator
                    current.type == T_MUL || current.type == T_DIV ||
                    current.type == T_LT || current.type == T_LE || 
                    current.type == T_GT || current.type == T_GE ||
                    current.type == T_EQ || current.type == T_NE ||
                    current.type == T_AND || current.type == T_OR)
            {
                parseTerm(); // Parse a simple term/expression
            }
        }
        else // If it does not start with a name
        {
            expression(); // Parse a general expression
        }
    }
    
    if (current.type == T_RPAREN) advance(); // Expect ')' to close for and move past it
    else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n"; // Report missing ')'
    
    // Body // Now parse the loop body
    if (current.type == T_LBRACE) // If '{' follows
    {
        block(); // Parse block as loop body
    }
    else // If no '{'
    {
        statement(); // Parse a single statement as loop body
    }
    semantic.logForLoop(current.line); // Log that we parsed a for loop
}

 // ------------------ Return Statement ------------------
 // [Syntax Analysis with Semantic Hooks] Parse return statements and note return type
void Parser::returnStatement() // This parses a return statement
{
    int retLine = current.line; // Save the line number of the return
    if (current.type == T_RETURN) advance(); // Move past 'return'
    else cout << "Syntax Error: expected 'return' at " << fileName << ":" << current.line << "\n"; // Report missing 'return'
    
    string retType; // This will store the type of the returned expression
    if (current.type != T_SEMICOLON) // If not just ';'
    {
        retType = expression(); // Parse the expression after return
    }
    
    if (current.type == T_SEMICOLON) advance(); // Expect and move past ';'
    else cout << "Syntax Error: expected ';' at " << fileName << ":" << current.line << "\n"; // Report missing ';'
    
    semantic.noteReturn(retLine, retType); // Record the return in semantics
    semantic.logReturn(retLine); // Log the return statement
}

 // ------------------ Expressions ------------------
 // [Syntax Analysis] Parse expressions with precedence
 // Recursive descent parser with operator precedence
 // Precedence (lowest to highest): logical (&&, ||) -> relational (<, <=, >, >=, ==, !=) -> term (+, -) -> factor (*, /) -> primary
string Parser::expression() // This starts expression parsing
{
    return parseLogical(); // Begin at the highest-level operators (logical)
}

 // [Syntax Analysis] Parse logical operators: &&, ||
string Parser::parseLogical() // This parses logical operations like && and ||
{
    string leftType = parseRelational(); // Parse the left side as a relational expression
    while (current.type == T_AND || current.type == T_OR) // While we see && or ||
    {
        Token op = current; // Save the operator token
        advance(); // Move past the operator
        string rightType = parseRelational(); // Parse the right side
        if (leftType != "bool" || rightType != "bool") // Check both sides are booleans
        {
            semantic.logError("Logical operator requires boolean operands", op.line); // Report type error
        }
        leftType = "bool"; // Result of logical operation is boolean
    }
    return leftType; // Return the resulting type
}

 // [Syntax Analysis] Parse relational operators: < <= > >= == !=
string Parser::parseRelational() // This parses comparisons like <, <=, >, >=, ==, !=
{
    string leftType = parseTerm(); // Parse the left side as a term
    while (current.type == T_LT || current.type == T_LE || current.type == T_GT ||  // While we see relational operators
           current.type == T_GE || current.type == T_EQ || current.type == T_NE)
    {
        Token op = current; // Save the operator
        advance(); // Move past the operator
        string rightType = parseTerm(); // Parse the right side
        if (leftType != rightType) // If types do not match
        {
            semantic.logError("Type mismatch in relational expression", op.line); // Report type mismatch
        }
        leftType = "bool"; // Result of comparison is boolean
    }
    return leftType; // Return the resulting type
}

 // [Syntax Analysis] Parse addition/subtraction
string Parser::parseTerm() // This parses + and - operations
{
    string leftType = parseFactor(); // Parse the left side as a factor
    while (current.type == T_PLUS || current.type == T_MINUS) // While we see + or -
    {
        Token op = current; // Save the operator
        advance(); // Move past the operator
        string rightType = parseFactor(); // Parse the right side
        if (!(leftType == "int" && rightType == "int")) // Both sides must be int
        {
            semantic.logError("Arithmetic operator requires integer operands", op.line); // Report type error
        }
        leftType = "int"; // Result of + or - is int
    }
    return leftType; // Return the resulting type
}

 // [Syntax Analysis] Parse multiplication/division
string Parser::parseFactor() // This parses * and / operations
{
    string leftType = parsePrimary(); // Parse the left side as a primary expression
    while (current.type == T_MUL || current.type == T_DIV) // While we see * or /
    {
        Token op = current; // Save the operator
        advance(); // Move past the operator
        string rightType = parsePrimary(); // Parse the right side
        if (!(leftType == "int" && rightType == "int")) // Both sides must be int
        {
            semantic.logError("Arithmetic operator requires integer operands", op.line); // Report type error
        }
        leftType = "int"; // Result of * or / is int
    }
    return leftType; // Return the resulting type
}

 // [Syntax Analysis with Semantic Hooks] Parse primary expressions and query semantic info
string Parser::parsePrimary()
{
    // Handle unary NOT operator
    if (current.type == T_NOT) {
        Token op = current;
        advance();
        string innerType = parsePrimary();
        if (innerType != "bool") {
            semantic.logError("Logical not '!' requires boolean operand", op.line);
        }
        return "bool";
    }

    if (current.type == T_NUMBER) {
        string t = "int";
        advance();
        return t;
    }
    if (current.type == T_CHAR_LITERAL) {
        string t = "char";
        advance();
        return t;
    }
    if (current.type == T_TRUE) {  // ADD THIS
        advance();
        return "bool";
    }
    if (current.type == T_FALSE) { // ADD THIS
        advance();
        return "bool";
    }
    else if (current.type == T_IDENTIFIER)
    {
        string name = current.value;
        int nameLine = current.line;
        advance();

        if (current.type == T_LBRACKET)
        {
            advance(); // skip '['
            if (current.type == T_NUMBER) {
                int idx = stoi(current.value);
                semantic.checkArrayIndex(name, idx, current.line);
                advance();
            } else {
                string indexType = expression(); // Get type of index expression
                semantic.checkArrayIndexType(indexType, current.line); // Check it is int
            }
            if (current.type == T_RBRACKET) advance();
            else cout << "Syntax Error: expected ']' at " << fileName << ":" << current.line << "\n";

            semantic.noteUse(name, nameLine, "array access");
            return semantic.getVariableType(name, nameLine);
        }
        // Function call: name(...)
        else if (current.type == T_LPAREN)
        {
            advance(); // '('
            vector<string> argTypes;
            if (current.type != T_RPAREN) {
                argTypes.push_back(expression());
                while (current.type == T_COMMA) {
                    advance();
                    argTypes.push_back(expression());
                }
            }
            if (current.type == T_RPAREN) advance();
            else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n";

            semantic.logFunctionCall(name, nameLine);
            semantic.checkFunctionCall(name, argTypes, nameLine); // verify call
            string ret = semantic.getFunctionReturnType(name, nameLine); // use declared return type
            return ret.empty() ? "int" : ret;
        }
        // Simple variable usage: name
        else
        {
            semantic.noteUse(name, nameLine, "expression");
            return semantic.getVariableType(name, nameLine);
        }
    }
    else if (current.type == T_LPAREN) // If we see '(' then it is a parenthesized expression
    {
        advance(); // Move past '('
        string t = expression(); // Parse the expression inside
        if (current.type == T_RPAREN) advance(); // Expect and move past ')'
        else cout << "Syntax Error: expected ')' at " << fileName << ":" << current.line << "\n"; // Report missing ')'
        return t; // Return the inner expression type
    }
    else {
        cout << "Syntax Error: expected number, identifier, character literal, 'true', 'false', or '(' at "
             << fileName << ":" << current.line << "\n";
        skipToRecovery();
        return "int";
    }
}

 // ------------------ Parse Entry ------------------
 // [Syntax Analysis with Semantic Hooks] Entry point to parse the entire program
void Parser::parse()
{
    program(); // Parse the whole program
    semantic.finalize(); // Print all logged info/warnings/errors
}
