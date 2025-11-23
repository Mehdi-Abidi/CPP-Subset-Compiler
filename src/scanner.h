#pragma once
#include "token.h"
#include <string>
using namespace std;

class Scanner {
private:
    string src;
    int pos;

public:
    Scanner(string source);
    Token getNextToken();
};
