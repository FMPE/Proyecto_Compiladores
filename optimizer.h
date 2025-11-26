#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>

// ============================================================================
// Optimización 1: DAG (Directed Acyclic Graph) para bloques básicos
// ============================================================================

enum class DAGNodeType {
    CONSTANT,    // Constante inmediata
    REGISTER,    // Registro
    OPERATION    // Operación aritmética
};

struct DAGNode {
    DAGNodeType type;
    std::string value;           // Valor/nombre del nodo
    std::string operation;       // Operación si es OPERATION (addq, subq, etc)
    std::vector<DAGNode*> children; // Hijos (operandos)
    std::vector<std::string> labels; // Etiquetas (registros/variables asignadas)
    
    DAGNode(DAGNodeType t, const std::string& v = "") 
        : type(t), value(v) {}
    
    bool operator==(const DAGNode& other) const {
        if (type != other.type || operation != other.operation) return false;
        if (children.size() != other.children.size()) return false;
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i] != other.children[i]) return false;
        }
        return true;
    }
};

class DAGOptimizer {
public:
    DAGOptimizer() = default;
    
    // Construye DAG a partir de un bloque básico de instrucciones
    void buildDAG(const std::vector<std::string>& instructions);
    
    // Genera código optimizado desde el DAG
    std::vector<std::string> generateOptimizedCode();
    
    // Limpia el DAG
    void clear();
    
private:
    std::vector<std::unique_ptr<DAGNode>> nodes;
    std::unordered_map<std::string, DAGNode*> registerMap; // registro -> nodo
    std::unordered_map<std::string, DAGNode*> constantMap; // constante -> nodo
    
    // Encuentra o crea nodo para constante
    DAGNode* getConstantNode(const std::string& value);
    
    // Encuentra o crea nodo para registro
    DAGNode* getRegisterNode(const std::string& reg);
    
    // Encuentra nodo existente para operación
    DAGNode* findOperationNode(const std::string& op, 
                               const std::vector<DAGNode*>& operands);
    
    // Crea nuevo nodo de operación
    DAGNode* createOperationNode(const std::string& op, 
                                 const std::vector<DAGNode*>& operands);
    
    // Parsea una instrucción de ensamblador
    void parseInstruction(const std::string& instr);
    
    // Extrae registro de un operando
    std::string extractRegister(const std::string& operand);
    
    // Verifica si es una constante inmediata
    bool isImmediate(const std::string& operand);
};

// ============================================================================
// Optimización 2: Peephole (mirilla) - optimizaciones locales
// ============================================================================

class PeepholeOptimizer {
public:
    PeepholeOptimizer() = default;
    
    // Aplica optimizaciones peephole a una lista de instrucciones
    std::vector<std::string> optimize(const std::vector<std::string>& instructions);
    
private:
    // Reglas de optimización específicas
    
    // Elimina movimientos redundantes: movq %rax, %rax
    bool eliminateRedundantMoves(std::vector<std::string>& instructions, size_t& i);
    
    // Combina operaciones con constantes: movq $5, %rax + addq $3, %rax -> movq $8, %rax
    bool combineConstantOperations(std::vector<std::string>& instructions, size_t& i);
    
    // Elimina código muerto: movq seguido de otro movq al mismo registro
    bool eliminateDeadCode(std::vector<std::string>& instructions, size_t& i);
    
    // Fortalecimiento de operaciones: addq $1 -> incq
    bool strengthReduction(std::vector<std::string>& instructions, size_t& i);
    
    // Propagación de constantes
    bool constantPropagation(std::vector<std::string>& instructions, size_t& i);
    
    // Optimización de comparaciones con 0
    bool optimizeZeroComparisons(std::vector<std::string>& instructions, size_t& i);
    
    // Funciones auxiliares
    bool isMovInstruction(const std::string& instr);
    bool isArithmeticInstruction(const std::string& instr);
    std::string getDestinationRegister(const std::string& instr);
    std::string getSourceOperand(const std::string& instr);
    bool isImmediate(const std::string& operand);
    long long getImmediateValue(const std::string& operand);
};

// ============================================================================
// Clase para dividir código en bloques básicos
// ============================================================================

struct BasicBlock {
    std::vector<std::string> instructions;
    std::string label;
    bool isEntry = false;
    bool isExit = false;
};

class BasicBlockAnalyzer {
public:
    // Divide el código en bloques básicos
    static std::vector<BasicBlock> identifyBasicBlocks(
        const std::vector<std::string>& allInstructions);
    
private:
    static bool isLabel(const std::string& instr);
    static bool isBranch(const std::string& instr);
    static bool isReturn(const std::string& instr);
};

// ============================================================================
// Clase Wrapper para aplicar todas las optimizaciones
// ============================================================================

class CodeOptimizer {
public:
    CodeOptimizer() : enableDAG(true), enablePeephole(true) {}
    
    // Aplica todas las optimizaciones habilitadas
    std::vector<std::string> optimizeCode(const std::vector<std::string>& code);
    
    // Configuración de optimizaciones
    void setDAGOptimization(bool enable) { enableDAG = enable; }
    void setPeepholeOptimization(bool enable) { enablePeephole = enable; }
    
    // Estadísticas
    struct Stats {
        int originalInstructions = 0;
        int optimizedInstructions = 0;
        int dagReductions = 0;
        int peepholeReductions = 0;
    };
    
    const Stats& getStats() const { return stats; }
    void resetStats() { stats = Stats(); }
    
private:
    bool enableDAG;
    bool enablePeephole;
    Stats stats;
    
    DAGOptimizer dagOpt;
    PeepholeOptimizer peepholeOpt;
};

#endif // OPTIMIZER_H