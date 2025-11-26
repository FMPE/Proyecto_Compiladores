#ifndef VISITOR_H
#define VISITOR_H
#include "ast.h"
#include "environment.h"
#include "optimizer.h"
#include <list>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

struct SymbolInfo {
    int offset = 0;
    Type::TType type = Type::NOTYPE;
    std::string typeName;
    bool isMutable = false;
    bool initialized = false;
};

// Estructura para el cache DAG de subexpresiones comunes
struct DAGCacheEntry {
    int offset;           // Offset en stack donde está guardado el resultado
    Type::TType type;     // Tipo del resultado
    std::string signature; // Firma de la expresión
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
    virtual int visit(StructDec* structDec) = 0;
    virtual int visit(TypeAlias* typeAlias) = 0;
    virtual int visit(StructInitExp* structInitExp) = 0;

    virtual int visit(BinaryExp* exp) = 0;
    virtual int visit(NumberExp* exp) = 0;
    virtual int visit(FloatExp* exp) = 0;
    virtual int visit(BoolExp* exp) = 0;
    virtual int visit(IdExp* exp) = 0;
    virtual int visit(FcallExp* exp) = 0;
    virtual int visit(ArrayAccessExp* exp) = 0;
    virtual int visit(FieldAccessExp* exp) = 0;
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
    int visit(StructDec* structDec) override;
    int visit(TypeAlias* typeAlias) override;
    int visit(StructInitExp* structInitExp) override;

    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(FloatExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(FcallExp* exp) override;
    int visit(ArrayAccessExp* exp) override;
    int visit(FieldAccessExp* exp) override;

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
    int visit(StructDec* structDec) override;
    int visit(TypeAlias* typeAlias) override;
    int visit(StructInitExp* structInitExp) override;

    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(FloatExp* exp) override;
    int visit(BoolExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(FcallExp* exp) override;
    int visit(ArrayAccessExp* exp) override;
    int visit(FieldAccessExp* exp) override;

    Type::TType lastType = Type::NOTYPE;
    
    // Métodos para optimización
    void enableOptimizations(bool enable) { optimizationsEnabled = enable; }
    void enableDAGOptimization(bool enable) { dagEnabled = enable; optimizer.setDAGOptimization(enable); }
    void enablePeepholeOptimization(bool enable) { optimizer.setPeepholeOptimization(enable); }
    void printOptimizationStats(std::ostream& os);

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
    
    // Sistema de optimización Peephole
    bool optimizationsEnabled = true;
    CodeOptimizer optimizer;
    std::ostringstream tempOutput;
    bool bufferingOutput = false;
    
    void startBuffering();
    void flushOptimizedBuffer();
    
    // ============================================
    // NUEVO: Sistema de optimización DAG
    // ============================================
    bool dagEnabled = true;
    
    // Cache de subexpresiones comunes: signature -> offset en stack
    std::unordered_map<std::string, DAGCacheEntry> dagCache;
    
    // Contador de subexpresiones reutilizadas (para estadísticas)
    int dagHits = 0;
    int dagMisses = 0;
    
    // Genera una firma única para una expresión
    std::string generateExprSignature(Exp* exp);
    
    // Busca una expresión en el cache DAG
    DAGCacheEntry* lookupDAGCache(const std::string& signature);
    
    // Guarda una expresión en el cache DAG
    void saveToDAGCache(const std::string& signature, int offset, Type::TType type);
    
    // Invalida entradas del cache que dependen de una variable
    void invalidateDAGCache(const std::string& varName);
    
    // Limpia todo el cache DAG
    void clearDAGCache();
};

#endif // VISITOR_H