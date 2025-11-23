#include <iostream>
#include <fstream>
#include <sstream>
#include "parser.h"
using namespace std;

int main() {
    ifstream file("input.txt");
    stringstream buffer;
    buffer << file.rdbuf();

    Scanner sc(buffer.str());
    Token t;
    do {
        t = sc.getNextToken();
        cout << "Token: " << t.value << endl;
    } while(t.type != T_EOF);

    Parser parser(sc);
    parser.parse();

    return 0;
}
