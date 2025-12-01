#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

struct SymbolInfo {
    string name;
    string type;      // "int", "char", "bool", "void"
    bool isArray = false;
    int arraySize = -1;
    int line = 0;
    string kind;      // "variable", "parameter", "function"
    bool initialized = false;
    int scopeDepth = 0;
};

struct FunctionInfo {
    string name;
    string returnType;
    vector<string> paramTypes;
    vector<string> paramNames;
    int line = 0;
    bool hasDefinition = false;
    bool hasReturnStatement = false;
};

class Semantic {
public:
    Semantic(const string& fileName);

    // Scope management
    void enterScope();
    void leaveScope();

    // Declarations
    bool declareVariable(const string& name, const string& type, bool isArray, int arraySize, int line, const string& kind);
    void noteInitialization(const string& name, int line);
    void noteUse(const string& name, int line, const string& context);

    // Functions
    bool declareFunction(const string& name,
                         const string& returnType,
                         const vector<string>& paramTypes,
                         const vector<string>& paramNames,
                         int line,
                         bool isDefinition);

    void enterFunctionBody(const string& name);
    void leaveFunctionBody();
    void noteReturn(int line, const string& returnedType);

    // Logging
    void logInfo(const string& message, int line);
    void logWarning(const string& message, int line);
    void logError(const string& message, int line);

    // Specific helpers
    void logDeclarationParsed(const string& name, const string& typeRepr, int line, const string& initializer = "");
    void logAssignment(const string& name, const string& exprRepr, int line);
    void logIfStatement(int line);
    void logForLoop(int line);
    void logReturn(int line);
    void logFunctionCall(const string& name, int line);

    // Expression type checking helpers
    string getVariableType(const string& name, int line);

    // Final reporting / checks
    void finalize();

private:
    string fileName;
    vector<map<string, SymbolInfo>> scopes;
    map<string, FunctionInfo> functions;
    string currentFunction;
    bool hasSemanticError = false;

    // main tracking
    bool mainSeen = false;
    int mainFirstLine = 0;
    int mainRedeclLine = 0;
    bool mainSignatureValid = true;
    string mainReturnType;

    vector<string> infoLogs;
    vector<string> warningLogs;
    vector<string> errorLogs;

    SymbolInfo* lookup(const string& name);
    SymbolInfo* lookupInCurrentScope(const string& name);

    void checkUninitializedAtUse(SymbolInfo* sym, int line, const string& context);
    void checkMainConstraints();
};
