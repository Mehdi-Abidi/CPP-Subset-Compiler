#include "scanner.h"
#include <cctype>

Scanner::Scanner(string source)
{
    src = source;
    pos = 0;
}

Token Scanner::getNextToken()
{
    while (pos < src.size() && isspace(src[pos]))
        pos++;

    if (pos >= src.size())
        return {T_EOF, ""};

    char c = src[pos];

    // Identifiers & Keywords
    if (isalpha(c))
    {
        string word = "";
        while (pos < src.size() && isalnum(src[pos]))
        {
            word += src[pos++];
        }
        if (word == "int")
            return {T_INT, word};
        if (word == "char")
            return {T_CHAR, word};
        if (word == "bool")
            return {T_BOOL, word};
        if (word == "double")
            return {T_DOUBLE, word};
        if (word == "float")
            return {T_FLOAT, word};
        if (word == "string")
            return {T_STRING, word};

        if (word == "if")
            return {T_IF, word};
        if (word == "else")
            return {T_ELSE, word};
        if (word == "for")
            return {T_FOR, word};
        if (word == "return")
            return {T_RETURN, word};
        if (word == "main")
            return {T_MAIN, word};
        return {T_IDENTIFIER, word};
    }
    if (c == ',')
    {
        pos++; // move past ','
        return {T_COMMA, ","};
    }
    if (c == '&' && pos + 1 < src.size() && src[pos + 1] == '&')
    {
        pos += 2;
        return {T_AND, "&&"};
    }
    if (c == '|' && pos + 1 < src.size() && src[pos + 1] == '|')
    {
        pos += 2;
        return {T_OR, "||"};
    }
    if (c == '!')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_NE, "!="};
        }
        else
        {
            pos++;
            return {T_NOT, "!"};
        }
    }

    // Check for multi-char operators first
    if (c == '=')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_EQ, "=="};
        }
        else
        {
            pos++;
            return {T_ASSIGN, "="};
        }
    }
    if (c == '!')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_NE, "!="};
        }
    }
    if (c == '<')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_LE, "<="};
        }
        else
        {
            pos++;
            return {T_LT, "<"};
        }
    }
    if (c == '>')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_GE, ">="};
        }
        else
        {
            pos++;
            return {T_GT, ">"};
        }
    }

    // Numbers
    if (isdigit(c))
    {
        string num = "";
        while (pos < src.size() && isdigit(src[pos]))
        {
            num += src[pos++];
        }
        return {T_NUMBER, num};
    }

    pos++;

    switch (c)
    {
    case '+':
        return {T_PLUS, "+"};
    case '-':
        return {T_MINUS, "-"};
    case '*':
        return {T_MUL, "*"};
    case '/':
        return {T_DIV, "/"};
    case '[':
        return {T_LBRACKET, "["};
    case ']':
        return {T_RBRACKET, "]"};
    case '=':
        return {T_ASSIGN, "="};
    case '(':
        return {T_LPAREN, "("};
    case ')':
        return {T_RPAREN, ")"};
    case '{':
        return {T_LBRACE, "{"};
    case '}':
        return {T_RBRACE, "}"};
    case ';':
        return {T_SEMICOLON, ";"};
    default:
        return {T_EOF, ""};
    }
}
