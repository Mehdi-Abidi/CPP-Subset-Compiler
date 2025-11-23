#include "scanner.h"
#include <cctype>

Scanner::Scanner(string source) {
    src = source;
    pos = 0;
}

Token Scanner::getNextToken() {
    while (pos < src.size() && isspace(src[pos])) pos++;

    if (pos >= src.size())
        return {T_EOF, ""};

    char c = src[pos];

    // Identifiers & Keywords
    if (isalpha(c)) {
        string word = "";
        while (pos < src.size() && isalnum(src[pos])) {
            word += src[pos++];
        }
        if (word == "int") return {T_INT, word};
        if (word == "char") return {T_CHAR, word};
        if (word == "bool") return {T_BOOL, word};
        if(word == "double") return {T_DOUBLE, word};
        if(word == "float") return {T_FLOAT, word};
        if(word == "string") return {T_STRING, word};

        if (word == "if") return {T_IF, word};
        if (word == "else") return {T_ELSE, word};
        if (word == "for") return {T_FOR, word};
        if (word == "return") return {T_RETURN, word};
        if (word == "main") return {T_MAIN, word};
        return {T_IDENTIFIER, word};
    }

    // Numbers
    if (isdigit(c)) {
        string num = "";
        while (pos < src.size() && isdigit(src[pos])) {
            num += src[pos++];
        }
        return {T_NUMBER, num};
    }

    pos++;

    switch (c) {
        case '+': return {T_PLUS, "+"};
        case '-': return {T_MINUS, "-"};
        case '*': return {T_MUL, "*"};
        case '/': return {T_DIV, "/"};
        case '=': return {T_ASSIGN, "="};
        case '(': return {T_LPAREN, "("};
        case ')': return {T_RPAREN, ")"};
        case '{': return {T_LBRACE, "{"};
        case '}': return {T_RBRACE, "}"};
        case ';': return {T_SEMICOLON, ";"};
        default:  return {T_EOF, ""};
    }
}
