#include "semantic.h"

Semantic::Semantic(const string& fileName) : fileName(fileName)
{
    // global scope
    scopes.push_back(map<string, SymbolInfo>());
}

void Semantic::enterScope()
{
    scopes.push_back(map<string, SymbolInfo>());
}

void Semantic::leaveScope()
{
    if (scopes.size() > 1)
    {
        scopes.pop_back();
    }
}

SymbolInfo* Semantic::lookup(const string& name)
{
    for (int i = (int)scopes.size() - 1; i >= 0; --i)
    {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end())
        {
            return &it->second;
        }
    }
    return nullptr;
}

SymbolInfo* Semantic::lookupInCurrentScope(const string& name)
{
    if (scopes.empty()) return nullptr;
    auto& cur = scopes.back();
    auto it = cur.find(name);
    if (it != cur.end()) return &it->second;
    return nullptr;
}

bool Semantic::declareVariable(const string& name, const string& type, bool isArray, int arraySize, int line, const string& kind)
{
    SymbolInfo* existingInCurrent = lookupInCurrentScope(name);
    SymbolInfo* existingInOuter = nullptr;
    if (!existingInCurrent)
    {
        existingInOuter = lookup(name);
    }

    if (existingInCurrent)
    {
        logError("Redeclaration of '" + name + "'", line);
        hasSemanticError = true;
        return false;
    }

    if (existingInOuter)
    {
        // Shadowing warning
        logWarning(
            "Shadowing of '" + name + "' declared at " + fileName + ":" + to_string(existingInOuter->line) +
            " by " + kind,
            line);
    }

    SymbolInfo sym;
    sym.name = name;
    sym.type = type;
    sym.isArray = isArray;
    sym.arraySize = arraySize;
    sym.line = line;
    sym.kind = kind;
    sym.initialized = false;
    sym.scopeDepth = (int)scopes.size() - 1;
    scopes.back()[name] = sym;
    return true;
}

void Semantic::noteInitialization(const string& name, int line)
{
    SymbolInfo* sym = lookup(name);
    if (!sym) return;
    sym->initialized = true;
}

void Semantic::checkUninitializedAtUse(SymbolInfo* sym, int line, const string& context)
{
    if (!sym) return;
    if (!sym->initialized)
    {
        logWarning("Use of potentially uninitialized variable '" + sym->name + "' in " + context, line);
    }
}

void Semantic::noteUse(const string& name, int line, const string& context)
{
    SymbolInfo* sym = lookup(name);
    if (!sym)
    {
        logError("Use of undeclared identifier '" + name + "'", line);
        hasSemanticError = true;
        return;
    }
    checkUninitializedAtUse(sym, line, context);
}

bool Semantic::declareFunction(const string& name,
                               const string& returnType,
                               const vector<string>& paramTypes,
                               const vector<string>& paramNames,
                               int line,
                               bool isDefinition)
{
    auto it = functions.find(name);
    if (it != functions.end())
    {
        FunctionInfo& existing = it->second;
        if (existing.returnType != returnType || existing.paramTypes != paramTypes)
        {
            logError("Conflicting declaration of function '" + name + "'", line);
            hasSemanticError = true;
            return false;
        }
        if (isDefinition && existing.hasDefinition)
        {
            logError("Multiple definitions of function '" + name +
                     "' (first defined at " + fileName + ":" + to_string(existing.line) +
                     ", redefined at " + fileName + ":" + to_string(line) + ")", line);
            hasSemanticError = true;
            return false;
        }
        if (isDefinition)
        {
            existing.hasDefinition = true;
            existing.line = line;
        }
    }
    else
    {
        FunctionInfo f;
        f.name = name;
        f.returnType = returnType;
        f.paramTypes = paramTypes;
        f.paramNames = paramNames;
        f.line = line;
        f.hasDefinition = isDefinition;
        f.hasReturnStatement = false;
        functions[name] = f;
    }

    if (name == "main")
    {
        if (!mainSeen)
        {
            mainSeen = true;
            mainFirstLine = line;
            mainReturnType = returnType;
        }
        else
        {
            mainRedeclLine = line;
        }
        if (!(returnType == "int" && paramTypes.empty()))
        {
            mainSignatureValid = false;
        }
    }

    return true;
}

void Semantic::enterFunctionBody(const string& name)
{
    currentFunction = name;
    enterScope(); // function-local scope (parameters + locals)
}

void Semantic::leaveFunctionBody()
{
    leaveScope();
    currentFunction.clear();
}

void Semantic::noteReturn(int line, const string& returnedType)
{
    if (currentFunction.empty()) return;
    FunctionInfo& f = functions[currentFunction];
    f.hasReturnStatement = true;
    if (f.returnType == "void")
    {
        logError("Return with a value in void function '" + f.name + "'", line);
        hasSemanticError = true;
    }
    else if (!returnedType.empty() && returnedType != f.returnType)
    {
        logError("Type mismatch in return statement of function '" + f.name + "'", line);
        hasSemanticError = true;
    }
}

void Semantic::logInfo(const string& message, int line)
{
    infoLogs.push_back("Info: " + message + " at " + fileName + ":" + to_string(line));
}

void Semantic::logWarning(const string& message, int line)
{
    warningLogs.push_back("Warning: " + message + " at " + fileName + ":" + to_string(line));
}

void Semantic::logError(const string& message, int line)
{
    errorLogs.push_back("Semantic Error: " + message + " at " + fileName + ":" + to_string(line));
    hasSemanticError = true;
}

void Semantic::logDeclarationParsed(const string& name, const string& typeRepr, int line, const string& initializer)
{
    string msg = "Declaration parsed: " + name + " (type=" + typeRepr + ")";
    if (!initializer.empty())
    {
        msg += " initializer=" + initializer;
    }
    logInfo(msg, line);
}

void Semantic::logAssignment(const string& name, const string& exprRepr, int line)
{
    logInfo("Assignment: " + name + " = " + exprRepr, line);
}

void Semantic::logIfStatement(int line)
{
    logInfo("If-else statement parsed", line);
}

void Semantic::logForLoop(int line)
{
    logInfo("For loop parsed", line);
}

void Semantic::logReturn(int line)
{
    logInfo("Return statement parsed", line);
}

void Semantic::logFunctionCall(const string& name, int line)
{
    logInfo("Function call: " + name, line);
}

string Semantic::getVariableType(const string& name, int line)
{
    SymbolInfo* sym = lookup(name);
    if (!sym)
    {
        logError("Use of undeclared identifier '" + name + "'", line);
        hasSemanticError = true;
        return "";
    }
    return sym->type;
}

void Semantic::checkMainConstraints()
{
    if (mainSeen && mainRedeclLine != 0)
    {
        errorLogs.push_back(
            "Semantic Error: Multiple definitions of function 'main' (first defined at " +
            fileName + ":" + to_string(mainFirstLine) + ", redefined at " +
            fileName + ":" + to_string(mainRedeclLine) + ").");
        hasSemanticError = true;
    }
    if (mainSeen && !mainSignatureValid)
    {
        errorLogs.push_back(
            "Semantic Error: 'main' must have signature 'int main()' at " +
            fileName + ":" + to_string(mainRedeclLine ? mainRedeclLine : mainFirstLine));
        hasSemanticError = true;
    }
}

void Semantic::finalize()
{
    // Check missing returns for non-void functions
    for (auto& p : functions)
    {
        FunctionInfo& f = p.second;
        if (f.returnType != "void" && f.hasDefinition && !f.hasReturnStatement)
        {
            warningLogs.push_back(
                "Warning: Control may reach end of non-void function '" + f.name + "' without a return statement at " +
                fileName + ":" + to_string(f.line));
        }
    }

    checkMainConstraints();

    // Print logs in order: Infos, Warnings, Errors
    for (const auto& s : infoLogs) std::cout << s << std::endl;
    for (const auto& s : warningLogs) std::cout << s << std::endl;
    for (const auto& s : errorLogs) std::cout << s << std::endl;

    if (hasSemanticError)
    {
        // In a fuller compiler we might throw/exit; here we just report.
    }
}
