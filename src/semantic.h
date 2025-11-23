#pragma once
#include <string>
#include <map>
using namespace std;

class Semantic {
public:
    map<string,string> symbolTable;
    void declare(string name, string type);
};
