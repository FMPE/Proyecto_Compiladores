#ifndef VISITOR_H
#define VISITOR_H
#include "ast.h"
#include "environment.h"
#include <list>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

struct SymbolInfo {
    int offset = 0;
    Type::TType type = Type::NOTYPE;
    bool isMutable = false;
    bool initialized = false;
};

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual int visit(Program* program) = 0;
    virtual int visit(FunDec* function) = 0;
    virtual int visit(Body* body) = 0;
    virtual int visit(BlockStm* block) = 0;
    virtual int visit(LetStm* letStmt) = 0;
    virtual int visit(IfStm* ifStmt) = 0;
    virtual int visit(WhileStm* whileStmt) = 0;
    virtual int visit(ForStm* forStmt) = 0;
    virtual int visit(PrintStm* printStmt) = 0;
    virtual int visit(AssignStm* assignStmt) = 0;
    virtual int visit(ReturnStm* returnStmt) = 0;
    virtual int visit(VarDec* varDec) = 0;

    virtual int visit(BinaryExp* exp) = 0;
    virtual int visit(NumberExp* exp) = 0;
    virtual int visit(BoolExp* exp) = 0;
    virtual int visit(IdExp* exp) = 0;
    virtual int visit(FcallExp* exp) = 0;
};

class TypeCheckerVisitor : public Visitor {
public:
    std::unordered_map<std::string, int> frameSlots;

    int analyze(Program* program);

    int visit(Program* program) override;
    int visit(FunDec* function) override;
    int visit(Body* body) override;
    int visit(BlockStm* block) override;
    int visit(LetStm* letStmt) override;
    int visit(IfStm* ifStmt) override;
    int visit(WhileStm* whileStmt) override;
    int visit(ForStm* forStmt) override;
    int visit(PrintStm* printStmt) override;
    int visit(AssignStm* assignStmt) override;
    int visit(ReturnStm* returnStmt) override;
    int visit(VarDec* varDec) override;

    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(FcallExp* exp) override;

private:
    int currentSlotCount = 0;
};

class GenCodeVisitor : public Visitor {
public:
    explicit GenCodeVisitor(std::ostream& output);

    int generar(Program* program);

    int visit(Program* program) override;
    int visit(FunDec* function) override;
    int visit(Body* body) override;
    int visit(BlockStm* block) override;
    int visit(LetStm* letStmt) override;
    int visit(IfStm* ifStmt) override;
    int visit(WhileStm* whileStmt) override;
    int visit(ForStm* forStmt) override;
    int visit(PrintStm* printStmt) override;
    int visit(AssignStm* assignStmt) override;
    int visit(ReturnStm* returnStmt) override;
    int visit(VarDec* varDec) override;

    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(FcallExp* exp) override;

private:
    std::ostream& out;
    TypeCheckerVisitor typeChecker;
    std::unordered_map<std::string, int> frameReservation;
    Environment<SymbolInfo> symbols;
    std::unordered_map<std::string, std::string> globalSymbols;

    int nextStackOffset = -8;
    int nextLabelId = 0;
    bool insideFunction = false;
    std::string currentFunctionName;
    std::string currentReturnLabel;

    std::string makeLabel(const std::string& base);
    SymbolInfo declareLocal(const std::string& name, const SymbolInfo& infoTemplate);
    const SymbolInfo* lookupSymbol(const std::string& name) const;
    SymbolInfo* lookupSymbol(const std::string& name);
};




#endif // VISITOR_H