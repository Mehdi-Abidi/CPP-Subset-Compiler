#include "scanner.h"
#include <cctype>

Scanner::Scanner(string source)
{
    src = source;
    pos = 0;
    line = 1;
}

Token Scanner::getNextToken()
{
    while (pos < src.size() && isspace(src[pos]))
    {
        if (src[pos] == '\n')
        {
            line++;
        }
        pos++;
    }

    if (pos >= src.size())
        return {T_EOF, "", line};

    char c = src[pos];

    // Identifiers & Keywords
    if (isalpha(c) || c == '_')
    {
        string word = "";
        while (pos < src.size() && (isalnum(src[pos]) || src[pos] == '_'))
        {
            word += src[pos++];
        }
        if (word == "int")
            return {T_INT, word, line};
        if (word == "char")
            return {T_CHAR, word, line};
        if (word == "bool")
            return {T_BOOL, word, line};
        if (word == "void")
            return {T_VOID, word, line};

        if (word == "if")
            return {T_IF, word, line};
        if (word == "else")
            return {T_ELSE, word, line};
        if (word == "for")
            return {T_FOR, word, line};
        if (word == "return")
            return {T_RETURN, word, line};
        if (word == "main")
            return {T_MAIN, word, line};
        return {T_IDENTIFIER, word, line};
    }
    if (c == ',')
    {
        pos++; // move past ','
        return {T_COMMA, ",", line};
    }
    if (c == '&' && pos + 1 < src.size() && src[pos + 1] == '&')
    {
        pos += 2;
        return {T_AND, "&&", line};
    }
    if (c == '|' && pos + 1 < src.size() && src[pos + 1] == '|')
    {
        pos += 2;
        return {T_OR, "||", line};
    }
    // Check for multi-char operators first
    if (c == '=')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_EQ, "==", line};
        }
        else
        {
            pos++;
            return {T_ASSIGN, "=", line};
        }
    }
    if (c == '!')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_NE, "!=", line};
        }
        else
        {
            pos++;
            return {T_NOT, "!", line};
        }
    }
    if (c == '<')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_LE, "<=", line};
        }
        else
        {
            pos++;
            return {T_LT, "<", line};
        }
    }
    if (c == '>')
    {
        if (pos + 1 < src.size() && src[pos + 1] == '=')
        {
            pos += 2;
            return {T_GE, ">=", line};
        }
        else
        {
            pos++;
            return {T_GT, ">", line};
        }
    }

    // Character literals
    if (c == '\'')
    {
        pos++; // skip opening quote
        if (pos < src.size())
        {
            char charVal = src[pos++];
            if (pos < src.size() && src[pos] == '\'')
            {
                pos++; // skip closing quote
                return {T_CHAR_LITERAL, string(1, charVal), line};
            }
            else
            {
                return {T_EOF, ""}; // error: unterminated character literal
            }
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
        return {T_NUMBER, num, line};
    }

    pos++;

    switch (c)
    {
    case '+':
        return {T_PLUS, "+", line};
    case '-':
        return {T_MINUS, "-", line};
    case '*':
        return {T_MUL, "*", line};
    case '/':
        return {T_DIV, "/", line};
    case '[':
        return {T_LBRACKET, "[", line};
    case ']':
        return {T_RBRACKET, "]", line};
    case '(':
        return {T_LPAREN, "(", line};
    case ')':
        return {T_RPAREN, ")", line};
    case '{':
        return {T_LBRACE, "{", line};
    case '}':
        return {T_RBRACE, "}", line};
    case ';':
        return {T_SEMICOLON, ";", line};
    default:
        return {T_EOF, "", line};
    }
}
