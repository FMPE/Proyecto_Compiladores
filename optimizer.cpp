#include "optimizer.h"
#include <sstream>
#include <algorithm>
#include <iostream>

// ============================================================================
// Implementación DAG Optimizer
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
    
    // Ignorar etiquetas y directivas
    if (op.empty() || op[0] == '.' || op.back() == ':') return;
    
    // MOVQ: destino = fuente
    if (op == "movq" || op == "movl") {
        std::string src, dst;
        iss >> src;
        // Eliminar coma
        if (!src.empty() && src.back() == ',') src.pop_back();
        iss >> dst;
        
        DAGNode* srcNode = nullptr;
        if (isImmediate(src)) {
            srcNode = getConstantNode(src);
        } else {
            srcNode = getRegisterNode(extractRegister(src));
        }
        
        std::string dstReg = extractRegister(dst);
        registerMap[dstReg] = srcNode;
        
        // Agregar etiqueta al nodo
        if (std::find(srcNode->labels.begin(), srcNode->labels.end(), dstReg) 
            == srcNode->labels.end()) {
            srcNode->labels.push_back(dstReg);
        }
        return;
    }
    
    // Operaciones aritméticas: addq, subq, imulq, etc.
    if (op == "addq" || op == "subq" || op == "imulq" || 
        op == "addl" || op == "subl" || op == "imull") {
        std::string src, dst;
        iss >> src;
        if (!src.empty() && src.back() == ',') src.pop_back();
        iss >> dst;
        
        std::string dstReg = extractRegister(dst);
        
        // Obtener nodo del valor previo de dst
        DAGNode* dstNode = getRegisterNode(dstReg);
        
        // Obtener nodo de src
        DAGNode* srcNode = nullptr;
        if (isImmediate(src)) {
            srcNode = getConstantNode(src);
        } else {
            srcNode = getRegisterNode(extractRegister(src));
        }
        
        // Buscar si ya existe esta operación
        std::string opName = op.substr(0, op.length() - 1); // Eliminar 'q' o 'l'
        if (opName == "imul") opName = "mul";
        
        DAGNode* resultNode = findOperationNode(opName, {dstNode, srcNode});
        
        if (!resultNode) {
            resultNode = createOperationNode(opName, {dstNode, srcNode});
        }
        
        // Actualizar mapeo de registro
        registerMap[dstReg] = resultNode;
        
        // Agregar etiqueta
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
    std::vector<std::string> result;
    
    // Generar código desde los nodos del DAG
    for (auto& node : nodes) {
        if (node->labels.empty()) continue; // No necesita materializarse
        
        std::string mainLabel = node->labels.back();
        
        if (node->type == DAGNodeType::CONSTANT) {
            result.push_back(" movq " + node->value + ", %" + mainLabel);
        } else if (node->type == DAGNodeType::OPERATION) {
            // Generar código para la operación
            if (node->children.size() == 2) {
                DAGNode* left = node->children[0];
                DAGNode* right = node->children[1];
                
                // Emitir instrucción optimizada
                std::string leftReg = left->labels.empty() ? "rax" : left->labels.back();
                std::string rightOp = right->type == DAGNodeType::CONSTANT ? 
                                     right->value : "%" + right->labels.back();
                
                std::string opcode = node->operation + "q";
                result.push_back(" " + opcode + " " + rightOp + ", %" + mainLabel);
            }
        }
    }
    
    return result;
}

void DAGOptimizer::clear() {
    nodes.clear();
    registerMap.clear();
    constantMap.clear();
}

std::string DAGOptimizer::extractRegister(const std::string& operand) {
    if (operand.empty()) return "";
    
    // Quitar '%' si existe
    std::string reg = operand;
    if (reg[0] == '%') reg = reg.substr(1);
    
    // Manejar direccionamiento indirecto: -8(%rbp) -> rbp
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
// Implementación Peephole Optimizer
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
            // Aplicar diferentes reglas de optimización
            if (eliminateRedundantMoves(result, i)) {
                changed = true;
                continue;
            }
            
            if (combineConstantOperations(result, i)) {
                changed = true;
                continue;
            }
            
            if (eliminateDeadCode(result, i)) {
                changed = true;
                continue;
            }
            
            if (strengthReduction(result, i)) {
                changed = true;
                continue;
            }
            
            if (optimizeZeroComparisons(result, i)) {
                changed = true;
                continue;
            }
        }
    }
    
    return result;
}

bool PeepholeOptimizer::eliminateRedundantMoves(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i >= instructions.size()) return false;
    
    std::istringstream iss(instructions[i]);
    std::string op, src, dst;
    iss >> op >> src >> dst;
    
    // movq %rax, %rax -> eliminar
    if ((op == "movq" || op == "movl") && !src.empty() && !dst.empty()) {
        if (src.back() == ',') src.pop_back();
        if (src == dst) {
            instructions.erase(instructions.begin() + i);
            return true;
        }
    }
    
    return false;
}

bool PeepholeOptimizer::combineConstantOperations(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i + 1 >= instructions.size()) return false;
    
    std::istringstream iss1(instructions[i]);
    std::string op1, src1, dst1;
    iss1 >> op1 >> src1 >> dst1;
    
    std::istringstream iss2(instructions[i + 1]);
    std::string op2, src2, dst2;
    iss2 >> op2 >> src2 >> dst2;
    
    // movq $5, %rax + addq $3, %rax -> movq $8, %rax
    if ((op1 == "movq" || op1 == "movl") && isImmediate(src1)) {
        if (src1.back() == ',') src1.pop_back();
        if (dst1.back() == ',') dst1.pop_back();
        
        if ((op2 == "addq" || op2 == "addl") && isImmediate(src2)) {
            if (src2.back() == ',') src2.pop_back();
            if (dst2.back() == ',') dst2.pop_back();
            
            if (dst1 == dst2) {
                long long val1 = getImmediateValue(src1);
                long long val2 = getImmediateValue(src2);
                long long result = val1 + val2;
                
                instructions[i] = " " + op1 + " $" + std::to_string(result) + ", " + dst1;
                instructions.erase(instructions.begin() + i + 1);
                return true;
            }
        }
        
        // Similar para subq
        if ((op2 == "subq" || op2 == "subl") && isImmediate(src2)) {
            if (src2.back() == ',') src2.pop_back();
            if (dst2.back() == ',') dst2.pop_back();
            
            if (dst1 == dst2) {
                long long val1 = getImmediateValue(src1);
                long long val2 = getImmediateValue(src2);
                long long result = val1 - val2;
                
                instructions[i] = " " + op1 + " $" + std::to_string(result) + ", " + dst1;
                instructions.erase(instructions.begin() + i + 1);
                return true;
            }
        }
    }
    
    return false;
}

bool PeepholeOptimizer::eliminateDeadCode(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i + 1 >= instructions.size()) return false;
    
    std::istringstream iss1(instructions[i]);
    std::string op1, src1, dst1;
    iss1 >> op1 >> src1 >> dst1;
    
    std::istringstream iss2(instructions[i + 1]);
    std::string op2, src2, dst2;
    iss2 >> op2 >> src2 >> dst2;
    
    // movq ..., %rax seguido de movq ..., %rax sin uso intermedio
    if (isMovInstruction(op1) && isMovInstruction(op2)) {
        if (dst1.back() == ',') dst1.pop_back();
        if (dst2.back() == ',') dst2.pop_back();
        
        if (dst1 == dst2) {
            // Eliminar la primera instrucción
            instructions.erase(instructions.begin() + i);
            return true;
        }
    }
    
    return false;
}

bool PeepholeOptimizer::strengthReduction(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i >= instructions.size()) return false;
    
    std::istringstream iss(instructions[i]);
    std::string op, src, dst;
    iss >> op >> src >> dst;
    
    if (src.back() == ',') src.pop_back();
    
    // addq $1, %rax -> incq %rax
    if ((op == "addq" || op == "addl") && src == "$1") {
        std::string newOp = (op == "addq") ? "incq" : "incl";
        instructions[i] = " " + newOp + " " + dst;
        return true;
    }
    
    // subq $1, %rax -> decq %rax
    if ((op == "subq" || op == "subl") && src == "$1") {
        std::string newOp = (op == "subq") ? "decq" : "decl";
        instructions[i] = " " + newOp + " " + dst;
        return true;
    }
    
    // imulq $2, %rax -> shlq $1, %rax (shift left = multiplicar por 2)
    if ((op == "imulq" || op == "imull") && src == "$2") {
        std::string newOp = (op == "imulq") ? "shlq" : "shll";
        instructions[i] = " " + newOp + " $1, " + dst;
        return true;
    }
    
    return false;
}

bool PeepholeOptimizer::optimizeZeroComparisons(
    std::vector<std::string>& instructions, size_t& i) {
    
    if (i + 1 >= instructions.size()) return false;
    
    std::istringstream iss1(instructions[i]);
    std::string op1, src1, dst1;
    iss1 >> op1 >> src1 >> dst1;
    
    // movq $0, %rax seguido de cmpq $0, %rax
    if ((op1 == "movq" || op1 == "movl")) {
        if (src1.back() == ',') src1.pop_back();
        if (dst1.back() == ',') dst1.pop_back();
        
        if (src1 == "$0" && i + 1 < instructions.size()) {
            std::istringstream iss2(instructions[i + 1]);
            std::string op2, src2, dst2;
            iss2 >> op2 >> src2 >> dst2;
            
            if (op2 == "cmpq" || op2 == "cmpl") {
                if (src2.back() == ',') src2.pop_back();
                if (dst2.back() == ',') dst2.pop_back();
                
                if (src2 == "$0" && dst1 == dst2) {
                    // testq %rax, %rax es más eficiente que cmpq $0, %rax
                    std::string newOp = (op2 == "cmpq") ? "testq" : "testl";
                    instructions[i + 1] = " " + newOp + " " + dst1 + ", " + dst1;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool PeepholeOptimizer::isMovInstruction(const std::string& instr) {
    return instr == "movq" || instr == "movl" || instr == "movb" || 
           instr == "movw";
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
// Implementación Basic Block Analyzer
// ============================================================================

std::vector<BasicBlock> BasicBlockAnalyzer::identifyBasicBlocks(
    const std::vector<std::string>& allInstructions) {
    
    std::vector<BasicBlock> blocks;
    BasicBlock currentBlock;
    
    for (const auto& instr : allInstructions) {
        std::string trimmed = instr;
        // Eliminar espacios iniciales
        size_t start = trimmed.find_first_not_of(" \t");
        if (start != std::string::npos) {
            trimmed = trimmed.substr(start);
        }
        
        if (trimmed.empty()) continue;
        
        // Si es una etiqueta, terminar bloque actual y empezar uno nuevo
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
        
        // Si es un salto o return, terminar bloque
        if (isBranch(trimmed) || isReturn(trimmed)) {
            blocks.push_back(currentBlock);
            currentBlock = BasicBlock();
        }
    }
    
    // Agregar último bloque si tiene contenido
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
// Implementación Code Optimizer (Wrapper)
// ============================================================================

std::vector<std::string> CodeOptimizer::optimizeCode(
    const std::vector<std::string>& code) {
    
    stats.originalInstructions = code.size();
    std::vector<std::string> result = code;
    
    // Aplicar optimizaciones Peephole primero (más agresivas)
    if (enablePeephole) {
        size_t beforePeephole = result.size();
        result = peepholeOpt.optimize(result);
        stats.peepholeReductions = beforePeephole - result.size();
    }
    
    // Luego aplicar DAG en bloques básicos
    if (enableDAG) {
        auto blocks = BasicBlockAnalyzer::identifyBasicBlocks(result);
        result.clear();
        
        for (auto& block : blocks) {
            // Separar instrucciones optimizables de etiquetas/directivas
            std::vector<std::string> optimizable;
            std::vector<std::string> nonOptimizable;
            
            for (const auto& instr : block.instructions) {
                std::string trimmed = instr;
                size_t start = trimmed.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    trimmed = trimmed.substr(start);
                }
                
                // No optimizar etiquetas, directivas, pushq/popq, call, cmp, test, saltos
                if (trimmed.empty() || trimmed[0] == '.' || trimmed.back() == ':' ||
                    trimmed.find("pushq") == 0 || trimmed.find("popq") == 0 ||
                    trimmed.find("call") == 0 || trimmed.find("cmpq") == 0 ||
                    trimmed.find("testq") == 0 || trimmed.find("j") == 0 ||
                    trimmed.find("leave") == 0 || trimmed.find("ret") == 0 ||
                    trimmed.find("leaq") == 0) {
                    
                    // Si hay instrucciones acumuladas, optimizarlas
                    if (!optimizable.empty()) {
                        dagOpt.clear();
                        dagOpt.buildDAG(optimizable);
                        auto optimized = dagOpt.generateOptimizedCode();
                        
                        if (!optimized.empty() && optimized.size() < optimizable.size()) {
                            result.insert(result.end(), optimized.begin(), optimized.end());
                            stats.dagReductions += optimizable.size() - optimized.size();
                        } else {
                            result.insert(result.end(), optimizable.begin(), optimizable.end());
                        }
                        optimizable.clear();
                    }
                    
                    result.push_back(instr);
                } else {
                    optimizable.push_back(instr);
                }
            }
            
            // Procesar instrucciones restantes
            if (!optimizable.empty()) {
                dagOpt.clear();
                dagOpt.buildDAG(optimizable);
                auto optimized = dagOpt.generateOptimizedCode();
                
                if (!optimized.empty() && optimized.size() < optimizable.size()) {
                    result.insert(result.end(), optimized.begin(), optimized.end());
                    stats.dagReductions += optimizable.size() - optimized.size();
                } else {
                    result.insert(result.end(), optimizable.begin(), optimizable.end());
                }
            }
        }
    }
    
    stats.optimizedInstructions = result.size();
    return result;
}