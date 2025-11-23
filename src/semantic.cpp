#include "semantic.h"
#include <iostream>
using namespace std;

void Semantic::declare(string name, string type) {
    if (symbolTable.count(name)) {
        cout << "Semantic Error: Redeclaration of " << name << endl;
        return;
    }
    symbolTable[name] = type;
}
