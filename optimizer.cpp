#include "optimizer.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <regex>

// ============================================================================
// ARQUITECTURA DE OPTIMIZACIONES DEL COMPILADOR
// ============================================================================
//
// Este compilador implementa DOS optimizaciones:
//
// 1. DAG (Directed Acyclic Graph) - Eliminación de subexpresiones comunes
//    ├── Ubicación: visitor.cpp (función generateExprSignature, lookupDAGCache, etc.)
//    ├── Momento: Durante la generación de código
//    └── Funcionamiento: Mantiene un cache de expresiones ya calculadas.
//        Cuando encuentra "a + b" por segunda vez, reutiliza el valor guardado.
//
// 2. PEEPHOLE (Mirilla) - Optimizaciones locales de assembly
//    ├── Ubicación: Este archivo (optimizer.cpp)
//    ├── Momento: Post-procesamiento del código generado
//    └── Funcionamiento: Busca patrones en el assembly y los reemplaza
//        por versiones más eficientes (ej: addq $1 → incq)
//
// ============================================================================

// ============================================================================
// DAGOptimizer - Clase auxiliar (la optimización real está en visitor.cpp)
// ============================================================================

void DAGOptimizer::buildDAG(const std::vector<std::string>& instructions) {
    for (const auto& instr : instructions) {
        parseInstruction(instr);
    }
}

void DAGOptimizer::parseInstruction(const std::string& instr) {
    std::istringstream iss(instr);
    std::string op;
    iss >> op;
    
    if (op.empty() || op[0] == '.' || op.back() == ':') return;
    
    if (op == "movq" || op == "movl") {
        std::string src, dst;
        iss >> src;
        if (!src.empty() && src.back() == ',') src.pop_back();
        iss >> dst;
        
        if (dst.find('(') != std::string::npos) {
            return;
        }
        
        DAGNode* srcNode = nullptr;
        if (isImmediate(src)) {
            srcNode = getConstantNode(src);
        } else {
            srcNode = getRegisterNode(extractRegister(src));
        }
        
        std::string dstReg = extractRegister(dst);
        registerMap[dstReg] = srcNode;
        
        if (std::find(srcNode->labels.begin(), srcNode->labels.end(), dstReg) 
            == srcNode->labels.end()) {
            srcNode->labels.push_back(dstReg);
        }
        return;
    }
    
    if (op == "addq" || op == "subq" || op == "imulq" || 
        op == "addl" || op == "subl" || op == "imull") {
        std::string src, dst;
        iss >> src;
        if (!src.empty() && src.back() == ',') src.pop_back();
        iss >> dst;
        
        if (dst.find('(') != std::string::npos) {
            return;
        }
        
        std::string dstReg = extractRegister(dst);
        DAGNode* dstNode = getRegisterNode(dstReg);
        
        DAGNode* srcNode = nullptr;
        if (isImmediate(src)) {
            srcNode = getConstantNode(src);
        } else {
            srcNode = getRegisterNode(extractRegister(src));
        }
        
        std::string opName = op.substr(0, op.length() - 1);
        if (opName == "imul") opName = "mul";
        
        DAGNode* resultNode = findOperationNode(opName, {dstNode, srcNode});
        
        if (!resultNode) {
            resultNode = createOperationNode(opName, {dstNode, srcNode});
        }
        
        registerMap[dstReg] = resultNode;
        
        if (std::find(resultNode->labels.begin(), resultNode->labels.end(), dstReg) 
            == resultNode->labels.end()) {
            resultNode->labels.push_back(dstReg);
        }
    }
}

DAGNode* DAGOptimizer::getConstantNode(const std::string& value) {
    auto it = constantMap.find(value);
    if (it != constantMap.end()) {
        return it->second;
    }
    
    auto node = std::make_unique<DAGNode>(DAGNodeType::CONSTANT, value);
    DAGNode* ptr = node.get();
    nodes.push_back(std::move(node));
    constantMap[value] = ptr;
    return ptr;
}

DAGNode* DAGOptimizer::getRegisterNode(const std::string& reg) {
    auto it = registerMap.find(reg);
    if (it != registerMap.end()) {
        return it->second;
    }
    
    auto node = std::make_unique<DAGNode>(DAGNodeType::REGISTER, reg);
    DAGNode* ptr = node.get();
    node->labels.push_back(reg);
    nodes.push_back(std::move(node));
    registerMap[reg] = ptr;
    return ptr;
}

DAGNode* DAGOptimizer::findOperationNode(const std::string& op, 
                                         const std::vector<DAGNode*>& operands) {
    for (auto& node : nodes) {
        if (node->type == DAGNodeType::OPERATION && 
            node->operation == op &&
            node->children == operands) {
            return node.get();
        }
    }
    return nullptr;
}

DAGNode* DAGOptimizer::createOperationNode(const std::string& op, 
                                           const std::vector<DAGNode*>& operands) {
    auto node = std::make_unique<DAGNode>(DAGNodeType::OPERATION);
    node->operation = op;
    node->children = operands;
    DAGNode* ptr = node.get();
    nodes.push_back(std::move(node));
    return ptr;
}

std::vector<std::string> DAGOptimizer::generateOptimizedCode() {
    return std::vector<std::string>();
}

void DAGOptimizer::clear() {
    nodes.clear();
    registerMap.clear();
    constantMap.clear();
}

std::string DAGOptimizer::extractRegister(const std::string& operand) {
    if (operand.empty()) return "";
    
    std::string reg = operand;
    if (reg[0] == '%') reg = reg.substr(1);
    
    size_t parenPos = reg.find('(');
    if (parenPos != std::string::npos) {
        size_t closePos = reg.find(')');
        reg = reg.substr(parenPos + 1, closePos - parenPos - 1);
        if (reg[0] == '%') reg = reg.substr(1);
    }
    
    return reg;
}

bool DAGOptimizer::isImmediate(const std::string& operand) {
    return !operand.empty() && operand[0] == '$';
}

// ============================================================================
// PEEPHOLE OPTIMIZER - OPTIMIZACIÓN ACTIVA
// ============================================================================
// Transforma patrones de instrucciones en versiones más eficientes:
//   - addq $1, %reg  →  incq %reg     (más compacto y rápido)
//   - subq $1, %reg  →  decq %reg     (más compacto y rápido)
//   - imulq $2, %reg →  shlq $1, %reg (shift es más rápido que mul)
//   - imulq $4, %reg →  shlq $2, %reg
//   - imulq $8, %reg →  shlq $3, %reg
//   - movq %reg, %reg → eliminado     (redundante)
//   - addq $0, %reg  →  eliminado     (no hace nada)
// ============================================================================

std::vector<std::string> PeepholeOptimizer::optimize(
    const std::vector<std::string>& instructions) {
    
    std::vector<std::string> result = instructions;
    bool changed = true;
    int passes = 0;
    const int MAX_PASSES = 5;
    
    while (changed && passes < MAX_PASSES) {
        changed = false;
        passes++;
        
        for (size_t i = 0; i < result.size(); ++i) {
            if (eliminateRedundantMoves(result, i)) {
                changed = true;
                continue;
            }
            
            if (strengthReduction(result, i)) {
                changed = true;
                continue;
            }
            
            if (eliminateDeadCode(result, i)) {
                changed = true;
                continue;
            }
            
            if (combineConstantOperations(result, i)) {
                changed = true;
                continue;
            }
            
            if (optimizeZeroComparisons(result, i)) {
                changed = true;
                continue;
            }
            
            if (constantPropagation(result, i)) {
                changed = true;
                continue;
            }
        }
    }
    
    // Eliminar líneas vacías (marcadas para eliminación)
    std::vector<std::string> cleaned;
    for (const auto& instr : result) {
        if (!instr.empty()) {
            cleaned.push_back(instr);
        }
    }
    
    return cleaned;
}

// Elimina movimientos redundantes: movq %rax, %rax
bool PeepholeOptimizer::eliminateRedundantMoves(
    std::vector<std::string>& instructions, size_t& i) {
    
    std::string& instr = instructions[i];
    std::istringstream iss(instr);
    std::string op, src, dst;
    iss >> op >> src >> dst;
    
    if (op == "movq" || op == "movl") {
        if (!src.empty() && src.back() == ',') src.pop_back();
        if (src == dst && src[0] == '%') {
            instructions[i] = "";
            return true;
        }
    }
    
    return false;
}

// Combina operaciones con constantes consecutivas
bool PeepholeOptimizer::combineConstantOperations(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i + 1 >= instructions.size()) return false;
    
    std::istringstream iss1(instructions[i]);
    std::string op1, src1, dst1;
    iss1 >> op1 >> src1 >> dst1;
    
    if (op1 == "movq" || op1 == "movl") {
        if (!src1.empty() && src1.back() == ',') src1.pop_back();
        
        if (isImmediate(src1)) {
            std::istringstream iss2(instructions[i + 1]);
            std::string op2, src2, dst2;
            iss2 >> op2 >> src2 >> dst2;
            
            if (!src2.empty() && src2.back() == ',') src2.pop_back();
            
            if ((op2 == "addq" || op2 == "addl") && isImmediate(src2) && dst1 == dst2) {
                long long val1 = getImmediateValue(src1);
                long long val2 = getImmediateValue(src2);
                instructions[i] = " " + op1 + " $" + std::to_string(val1 + val2) + ", " + dst1;
                instructions[i + 1] = "";
                return true;
            }
            
            if ((op2 == "subq" || op2 == "subl") && isImmediate(src2) && dst1 == dst2) {
                long long val1 = getImmediateValue(src1);
                long long val2 = getImmediateValue(src2);
                instructions[i] = " " + op1 + " $" + std::to_string(val1 - val2) + ", " + dst1;
                instructions[i + 1] = "";
                return true;
            }
        }
    }
    
    return false;
}

// Elimina código muerto
bool PeepholeOptimizer::eliminateDeadCode(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i + 1 >= instructions.size()) return false;
    
    std::istringstream iss1(instructions[i]);
    std::string op1, src1, dst1;
    iss1 >> op1 >> src1 >> dst1;
    
    if (op1 == "movq" || op1 == "movl") {
        if (!src1.empty() && src1.back() == ',') src1.pop_back();
        
        if (dst1.find('(') != std::string::npos) return false;
        
        std::istringstream iss2(instructions[i + 1]);
        std::string op2, src2, dst2;
        iss2 >> op2 >> src2 >> dst2;
        
        if ((op2 == "movq" || op2 == "movl") && dst1 == dst2) {
            instructions[i] = "";
            return true;
        }
    }
    
    return false;
}

// STRENGTH REDUCTION - Reemplaza operaciones costosas por equivalentes rápidas
bool PeepholeOptimizer::strengthReduction(
    std::vector<std::string>& instructions, size_t& i) {
    
    std::istringstream iss(instructions[i]);
    std::string op, src, dst;
    iss >> op >> src >> dst;
    
    if (!src.empty() && src.back() == ',') src.pop_back();
    
    // addq $1, %rax → incq %rax
    if ((op == "addq" || op == "addl") && src == "$1") {
        std::string newOp = (op == "addq") ? "incq" : "incl";
        instructions[i] = " " + newOp + " " + dst;
        return true;
    }
    
    // subq $1, %rax → decq %rax
    if ((op == "subq" || op == "subl") && src == "$1") {
        std::string newOp = (op == "subq") ? "decq" : "decl";
        instructions[i] = " " + newOp + " " + dst;
        return true;
    }
    
    // imulq $2, %rax → shlq $1, %rax
    if ((op == "imulq" || op == "imull") && src == "$2") {
        std::string newOp = (op == "imulq") ? "shlq" : "shll";
        instructions[i] = " " + newOp + " $1, " + dst;
        return true;
    }
    
    // imulq $4, %rax → shlq $2, %rax
    if ((op == "imulq" || op == "imull") && src == "$4") {
        std::string newOp = (op == "imulq") ? "shlq" : "shll";
        instructions[i] = " " + newOp + " $2, " + dst;
        return true;
    }
    
    // imulq $8, %rax → shlq $3, %rax
    if ((op == "imulq" || op == "imull") && src == "$8") {
        std::string newOp = (op == "imulq") ? "shlq" : "shll";
        instructions[i] = " " + newOp + " $3, " + dst;
        return true;
    }
    
    // addq $0, %rax → eliminar
    if ((op == "addq" || op == "addl" || op == "subq" || op == "subl") && src == "$0") {
        instructions[i] = "";
        return true;
    }
    
    // imulq $1, %rax → eliminar
    if ((op == "imulq" || op == "imull") && src == "$1") {
        instructions[i] = "";
        return true;
    }
    
    return false;
}

bool PeepholeOptimizer::constantPropagation(
    std::vector<std::string>& instructions, size_t& i) {
    return false;
}

// cmpq $0, %rax → testq %rax, %rax (más eficiente)
bool PeepholeOptimizer::optimizeZeroComparisons(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i + 1 >= instructions.size()) return false;
    
    std::istringstream iss1(instructions[i]);
    std::string op1, src1, dst1;
    iss1 >> op1 >> src1 >> dst1;
    
    if (op1 == "cmpq" || op1 == "cmpl") {
        if (!src1.empty() && src1.back() == ',') src1.pop_back();
        
        if (src1 == "$0") {
            std::string newOp = (op1 == "cmpq") ? "testq" : "testl";
            instructions[i] = " " + newOp + " " + dst1 + ", " + dst1;
            return true;
        }
    }
    
    return false;
}

bool PeepholeOptimizer::isMovInstruction(const std::string& instr) {
    return instr == "movq" || instr == "movl" || instr == "movb" || instr == "movw";
}

bool PeepholeOptimizer::isArithmeticInstruction(const std::string& instr) {
    return instr == "addq" || instr == "subq" || instr == "imulq" || 
           instr == "addl" || instr == "subl" || instr == "imull";
}

bool PeepholeOptimizer::isImmediate(const std::string& operand) {
    return !operand.empty() && operand[0] == '$';
}

long long PeepholeOptimizer::getImmediateValue(const std::string& operand) {
    if (!isImmediate(operand)) return 0;
    return std::stoll(operand.substr(1));
}

// ============================================================================
// Basic Block Analyzer
// ============================================================================

std::vector<BasicBlock> BasicBlockAnalyzer::identifyBasicBlocks(
    const std::vector<std::string>& allInstructions) {
    
    std::vector<BasicBlock> blocks;
    BasicBlock currentBlock;
    
    for (const auto& instr : allInstructions) {
        std::string trimmed = instr;
        size_t start = trimmed.find_first_not_of(" \t");
        if (start != std::string::npos) {
            trimmed = trimmed.substr(start);
        }
        
        if (trimmed.empty()) continue;
        
        if (isLabel(trimmed)) {
            if (!currentBlock.instructions.empty()) {
                blocks.push_back(currentBlock);
                currentBlock = BasicBlock();
            }
            currentBlock.label = trimmed;
            currentBlock.instructions.push_back(instr);
            continue;
        }
        
        currentBlock.instructions.push_back(instr);
        
        if (isBranch(trimmed) || isReturn(trimmed)) {
            blocks.push_back(currentBlock);
            currentBlock = BasicBlock();
        }
    }
    
    if (!currentBlock.instructions.empty()) {
        blocks.push_back(currentBlock);
    }
    
    return blocks;
}

bool BasicBlockAnalyzer::isLabel(const std::string& instr) {
    return !instr.empty() && instr.back() == ':';
}

bool BasicBlockAnalyzer::isBranch(const std::string& instr) {
    std::istringstream iss(instr);
    std::string op;
    iss >> op;
    return op == "jmp" || op == "je" || op == "jne" || op == "jl" || 
           op == "jg" || op == "jle" || op == "jge" || op == "call";
}

bool BasicBlockAnalyzer::isReturn(const std::string& instr) {
    std::istringstream iss(instr);
    std::string op;
    iss >> op;
    return op == "ret" || op == "leave";
}

// ============================================================================
// CodeOptimizer - Wrapper principal que coordina las optimizaciones
// ============================================================================

std::vector<std::string> CodeOptimizer::optimizeCode(
    const std::vector<std::string>& code) {
    
    stats.originalInstructions = code.size();
    std::vector<std::string> result = code;
    
    // Aplicar optimización Peephole (post-procesamiento)
    if (enablePeephole) {
        size_t beforePeephole = result.size();
        result = peepholeOpt.optimize(result);
        stats.peepholeReductions = beforePeephole - result.size();
    }
    
    // NOTA: La optimización DAG se realiza en visitor.cpp durante
    // la generación de código (ver funciones generateExprSignature,
    // lookupDAGCache, saveToDAGCache en visitor.cpp)
    
    stats.optimizedInstructions = result.size();
    return result;
}