#include <iostream>
#include <fstream>
#include <sstream>
#include "parser.h"
using namespace std;

int main() {
    ifstream file("input.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open input.txt" << endl;
        return 1;
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    Scanner sc(buffer.str());
    Parser parser(sc, "input.txt");
    parser.parse();

    return 0;
}
