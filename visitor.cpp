#include "visitor.h"

#include "ast.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <cstring> // For memcpy
#include <cstdint>

using std::string;
using std::vector;

namespace {
const vector<string> kArgRegisters = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

Type::TType resolve_type(const string& name) {
	auto tt = Type::string_to_type(name);
	return tt;
}

struct StructLayout {
    int size;
    std::unordered_map<std::string, int> offsets;
    std::unordered_map<std::string, std::string> types;
};
static std::unordered_map<std::string, StructLayout> globalStructLayouts;
static std::unordered_map<std::string, std::string> globalTypeAliases;

string resolve_alias(string name) {
    while (globalTypeAliases.count(name)) {
        name = globalTypeAliases[name];
    }
    return name;
}
}

// -----------------------------------------------------------------------------
// Accept implementations (kept alongside visitor logic for clarity)
// -----------------------------------------------------------------------------

// Accept implementations moved to ast.cpp


// -----------------------------------------------------------------------------
// TypeCheckerVisitor implementation (frame size planning)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::analyze(Program* program) {
	frameSlots.clear();
	currentSlotCount = 0;
	program->accept(this);
	return 0;
}

int TypeCheckerVisitor::visit(Program* program) {
    for (auto typeAlias : program->talist) {
        if (typeAlias) {
            typeAlias->accept(this);
        }
    }
    for (auto structDecl : program->sdlist) {
        if (structDecl) {
            structDecl->accept(this);
        }
    }
	for (auto globalDecl : program->vdlist) {
		if (globalDecl) {
			globalDecl->accept(this);
		}
	}

	for (auto functionDecl : program->fdlist) {
		if (functionDecl) {
			functionDecl->accept(this);
		}
	}
	return 0;
}

int TypeCheckerVisitor::visit(FunDec* function) {
	currentSlotCount = static_cast<int>(function->Nparametros.size());

	if (function->cuerpo) {
		function->cuerpo->accept(this);
	}

	frameSlots[function->nombre] = currentSlotCount;
	currentSlotCount = 0;
	return 0;
}

int TypeCheckerVisitor::visit(Body* body) {
	for (auto decl : body->vdlist) {
		if (decl) {
			decl->accept(this);
		}
	}
	for (auto stmt : body->stmlist) {
		if (stmt) {
			stmt->accept(this);
		}
	}
	return 0;
}

int TypeCheckerVisitor::visit(BlockStm* block) {
	for (auto stmt : block->statements) {
		if (stmt) {
			stmt->accept(this);
		}
	}
	return 0;
}

int TypeCheckerVisitor::visit(LetStm* letStmt) {
    int slots = 1;
    string typeName = resolve_alias(letStmt->type_name);
    if (typeName.find("[") != string::npos) {
        size_t open = typeName.find("[");
        size_t close = typeName.find("]");
        int count = stoi(typeName.substr(open + 1, close - open - 1));
        int sizeBytes = count * 4;
        slots = (sizeBytes + 7) / 8;
    } else if (globalStructLayouts.count(typeName)) {
        int sizeBytes = globalStructLayouts[typeName].size;
        slots = (sizeBytes + 7) / 8;
    }
	currentSlotCount += slots;
	if (letStmt->init) {
		letStmt->init->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(IfStm* ifStmt) {
	if (ifStmt->condition) {
		ifStmt->condition->accept(this);
	}
	if (ifStmt->thenBlock) {
		ifStmt->thenBlock->accept(this);
	}
	if (ifStmt->elseBlock) {
		ifStmt->elseBlock->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(WhileStm* whileStmt) {
	if (whileStmt->condition) {
		whileStmt->condition->accept(this);
	}
	if (whileStmt->body) {
		whileStmt->body->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(ForStm* forStmt) {
	++currentSlotCount; // iterador
	if (forStmt->start) {
		forStmt->start->accept(this);
	}
	if (forStmt->end) {
		forStmt->end->accept(this);
	}
	if (forStmt->body) {
		forStmt->body->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(PrintStm* printStmt) {
	if (printStmt->e) {
		printStmt->e->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(AssignStm* assignStmt) {
	if (assignStmt->e) {
		assignStmt->e->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(ReturnStm* returnStmt) {
	if (returnStmt->e) {
		returnStmt->e->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(VarDec* varDec) {
	currentSlotCount += static_cast<int>(varDec->variables.size());
	return 0;
}

int TypeCheckerVisitor::visit(BinaryExp* exp) {
	if (exp->left) {
		exp->left->accept(this);
	}
	if (exp->right) {
		exp->right->accept(this);
	}
	return 0;
}

int TypeCheckerVisitor::visit(NumberExp* /*exp*/) { return 0; }
int TypeCheckerVisitor::visit(BoolExp* /*exp*/) { return 0; }
int TypeCheckerVisitor::visit(IdExp* /*exp*/) { return 0; }
int TypeCheckerVisitor::visit(FloatExp* /*exp*/) { return 0; }

int TypeCheckerVisitor::visit(FcallExp* exp) {
	for (auto arg : exp->argumentos) {
		if (arg) {
			arg->accept(this);
		}
	}
	return 0;
}

int TypeCheckerVisitor::visit(StructDec* sd) {
    StructLayout layout;
    int currentOffset = 0;
    for (auto& field : sd->fields) {
        layout.offsets[field.first] = currentOffset;
        layout.types[field.first] = field.second;
        string type = resolve_alias(field.second); // Resolve alias
        int size = 8; 
        if (type.find("[") != string::npos) {
             size_t open = type.find("[");
             size_t close = type.find("]");
             int count = stoi(type.substr(open + 1, close - open - 1));
             size = count * 4; 
        } else if (type == "i32" || type == "bool" || type == "u32" || type == "f32") {
             size = 4;
        }
        layout.offsets[field.first] = currentOffset;
        currentOffset += size;
    }
    layout.size = currentOffset;
    globalStructLayouts[sd->name] = layout;
    return 0;
}

int TypeCheckerVisitor::visit(TypeAlias* ta) {
    globalTypeAliases[ta->alias] = ta->type;
    return 0;
}

int TypeCheckerVisitor::visit(ArrayAccessExp* e) { return 0; }
int TypeCheckerVisitor::visit(FieldAccessExp* e) { return 0; }
int TypeCheckerVisitor::visit(StructInitExp* exp) {
    string resolvedName = resolve_alias(exp->name);
    if (globalStructLayouts.count(resolvedName)) {
        int size = globalStructLayouts[resolvedName].size;
        int slots = (size + 7) / 8;
        currentSlotCount += slots;
    }
    for (auto& field : exp->fields) {
        field.second->accept(this);
    }
    return 0;
}

// -----------------------------------------------------------------------------
// GenCodeVisitor helper utilities
// -----------------------------------------------------------------------------

GenCodeVisitor::GenCodeVisitor(std::ostream& output)
	: out(output) {}

string GenCodeVisitor::makeLabel(const string& base) {
	return ".L_" + base + "_" + std::to_string(nextLabelId++);
}

SymbolInfo GenCodeVisitor::declareLocal(const string& name, const SymbolInfo& infoTemplate) {
	SymbolInfo info = infoTemplate;
	info.offset = nextStackOffset;
	nextStackOffset -= 8;
	symbols.declare(name, info);
	return info;
}

const SymbolInfo* GenCodeVisitor::lookupSymbol(const string& name) const {
	return symbols.lookup(name);
}

SymbolInfo* GenCodeVisitor::lookupSymbol(const string& name) {
	return symbols.lookup(name);
}

// -----------------------------------------------------------------------------
// GenCodeVisitor implementation
// -----------------------------------------------------------------------------

int GenCodeVisitor::generar(Program* program) {
	frameReservation.clear();
	typeChecker.analyze(program);
	frameReservation = typeChecker.frameSlots;
	return program->accept(this);
}

int GenCodeVisitor::visit(Program* program) {
	out << ".data\n";
	out << "print_fmt: .string \"%ld \\n\"\n";
    out << "print_float_fmt: .string \"%f \\n\"\n";

	for (auto globalDecl : program->vdlist) {
		if (globalDecl) {
			globalDecl->accept(this);
		}
	}

	for (auto it = globalSymbols.begin(); it != globalSymbols.end(); ++it) {
		out << it->second << ": .quad 0\n";
	}

	out << ".text\n";

    for (auto typeAlias : program->talist) {
        if (typeAlias) {
            typeAlias->accept(this);
        }
    }

    for (auto structDecl : program->sdlist) {
        if (structDecl) {
            structDecl->accept(this);
        }
    }

	for (auto functionDecl : program->fdlist) {
		if (functionDecl) {
			functionDecl->accept(this);
		}
	}

	out << ".section .note.GNU-stack,\"\",@progbits\n";
	return 0;
}

int GenCodeVisitor::visit(FunDec* function) {
	insideFunction = true;
	symbols.clear();
	symbols.push_scope();
	nextStackOffset = -8;

	currentFunctionName = function->nombre;
	currentReturnLabel = ".L_return_" + function->nombre;

	out << ".globl " << function->nombre << "\n";
	out << function->nombre << ":\n";
	out << " pushq %rbp\n";
	out << " movq %rsp, %rbp\n";

	int reservedSlots = 0;
	auto it = frameReservation.find(function->nombre);
	if (it != frameReservation.end()) {
		reservedSlots = it->second;
	}
	int frameBytes = reservedSlots * 8;
	if (frameBytes > 0) {
		out << " subq $" << frameBytes << ", %rsp\n";
	}

	auto paramCount = function->Nparametros.size();
	for (std::size_t idx = 0; idx < paramCount && idx < kArgRegisters.size(); ++idx) {
		SymbolInfo tmpl;
		tmpl.isMutable = false;
		tmpl.initialized = true;
		tmpl.type = resolve_type(function->Tparametros[idx]);
        tmpl.typeName = function->Tparametros[idx];
		SymbolInfo info = declareLocal(function->Nparametros[idx], tmpl);
		out << " movq " << kArgRegisters[idx] << ", " << info.offset << "(%rbp)\n";
	}

	if (function->cuerpo) {
		function->cuerpo->accept(this);
	}

	out << " movq $0, %rax\n";
	out << currentReturnLabel << ":\n";
	out << " leave\n";
	out << " ret\n";

	symbols.clear();
	insideFunction = false;
	currentFunctionName.clear();
	currentReturnLabel.clear();
	return 0;
}

int GenCodeVisitor::visit(Body* body) {
	for (auto decl : body->vdlist) {
		if (decl) {
			decl->accept(this);
		}
	}
	for (auto stmt : body->stmlist) {
		if (stmt) {
			stmt->accept(this);
		}
	}
	return 0;
}

int GenCodeVisitor::visit(BlockStm* block) {
	symbols.push_scope();
	for (auto stmt : block->statements) {
		if (stmt) {
			stmt->accept(this);
		}
	}
	symbols.pop_scope();
	return 0;
}

int GenCodeVisitor::visit(LetStm* letStmt) {
	if (!insideFunction) {
		if (globalSymbols.find(letStmt->name) == globalSymbols.end()) {
			globalSymbols.emplace(letStmt->name, letStmt->name);
		}
		return 0;
	}

	SymbolInfo tmpl;
	tmpl.isMutable = letStmt->mutable_flag;
	tmpl.initialized = letStmt->init != nullptr;
	tmpl.type = resolve_type(letStmt->type_name);
    tmpl.typeName = letStmt->type_name;

    int size = 8;
    string typeName = resolve_alias(letStmt->type_name);
    if (typeName.find("[") != string::npos) {
        size_t open = typeName.find("[");
        size_t close = typeName.find("]");
        int count = stoi(typeName.substr(open + 1, close - open - 1));
        size = count * 4; 
    } else if (globalStructLayouts.count(typeName)) {
        size = globalStructLayouts[typeName].size;
    } else if (tmpl.type == Type::F32 || tmpl.type == Type::I32 || tmpl.type == Type::U32) {
        size = 4;
    }

    // Allocate space
    // Round up size to multiple of 8 to maintain stack alignment
    int alignedSize = (size + 7) / 8 * 8;
    
    // Calculate offset: use the space ending at current nextStackOffset
    // Formula: base = nextStackOffset - alignedSize + 8
    // (Assuming nextStackOffset points to the start of the *next* available 8-byte slot, e.g., -8)
    int offset = nextStackOffset - alignedSize + 8;
    nextStackOffset -= alignedSize;
    
    tmpl.offset = offset;
    symbols.declare(letStmt->name, tmpl);

	if (letStmt->init) {
		letStmt->init->accept(this);
        Type::TType rhsType = lastType;

        if (tmpl.type == Type::F32 && (rhsType == Type::F64 || rhsType == Type::NOTYPE)) {
             out << " movq %rax, %xmm0\n";
             out << " cvtsd2ss %xmm0, %xmm0\n";
             out << " movd %xmm0, %eax\n";
             out << " movl %eax, " << tmpl.offset << "(%rbp)\n";
        } else if (size <= 8) {
            if (size == 4) {
                 out << " movl %eax, " << tmpl.offset << "(%rbp)\n";
            } else {
		         out << " movq %rax, " << tmpl.offset << "(%rbp)\n";
            }
        } else {
            // Copy from source address in %rax to destination address at tmpl.offset(%rbp)
            out << " movq %rax, %rsi\n"; // Source
            out << " leaq " << tmpl.offset << "(%rbp), %rdi\n"; // Destination
            out << " movq $" << size << ", %rcx\n"; // Count
            out << " rep movsb\n";
        }
	} else {
        if (size <= 8) {
		    out << " movq $0, " << tmpl.offset << "(%rbp)\n";
        } else {
            // Zero init large area?
            // Optional.
        }
	}

	return 0;
}

int GenCodeVisitor::visit(IfStm* ifStmt) {
	string elseLabel = makeLabel("else");
	string endLabel = makeLabel("endif");

	ifStmt->condition->accept(this);
	out << " cmpq $0, %rax\n";
	out << " je " << elseLabel << "\n";

	if (ifStmt->thenBlock) {
		ifStmt->thenBlock->accept(this);
	}
	out << " jmp " << endLabel << "\n";

	out << elseLabel << ":\n";
	if (ifStmt->elseBlock) {
		ifStmt->elseBlock->accept(this);
	}
	out << endLabel << ":\n";
	return 0;
}

int GenCodeVisitor::visit(WhileStm* whileStmt) {
	string startLabel = makeLabel("while_begin");
	string endLabel = makeLabel("while_end");

	out << startLabel << ":\n";
	whileStmt->condition->accept(this);
	out << " cmpq $0, %rax\n";
	out << " je " << endLabel << "\n";

	if (whileStmt->body) {
		whileStmt->body->accept(this);
	}

	out << " jmp " << startLabel << "\n";
	out << endLabel << ":\n";
	return 0;
}

int GenCodeVisitor::visit(ForStm* forStmt) {
	symbols.push_scope();

	SymbolInfo tmpl;
	tmpl.isMutable = true;
	tmpl.initialized = true;
	tmpl.type = Type::I64;
	SymbolInfo iterInfo = declareLocal(forStmt->iteratorName, tmpl);

	if (forStmt->start) {
		forStmt->start->accept(this);
	} else {
		out << " movq $0, %rax\n";
	}
	out << " movq %rax, " << iterInfo.offset << "(%rbp)\n";

	string loopLabel = makeLabel("for_begin");
	string endLabel = makeLabel("for_end");

	out << loopLabel << ":\n";
	if (forStmt->end) {
		forStmt->end->accept(this);
	} else {
		out << " movq $0, %rax\n";
	}
	out << " movq %rax, %rcx\n";
	out << " movq " << iterInfo.offset << "(%rbp), %rax\n";
	out << " cmpq %rcx, %rax\n";
	out << " jge " << endLabel << "\n";

	if (forStmt->body) {
		forStmt->body->accept(this);
	}

	out << " movq " << iterInfo.offset << "(%rbp), %rax\n";
	out << " addq $1, %rax\n";
	out << " movq %rax, " << iterInfo.offset << "(%rbp)\n";
	out << " jmp " << loopLabel << "\n";
	out << endLabel << ":\n";

	symbols.pop_scope();
	return 0;
}

int GenCodeVisitor::visit(PrintStm* printStmt) {
	if (!printStmt->e) {
		out << " movq $0, %rax\n";
	} else {
		printStmt->e->accept(this);
	}

    if (lastType == Type::F32 || lastType == Type::F64) {
        out << " movq %rax, %xmm0\n";
        if (lastType == Type::F32) {
             out << " cvtss2sd %xmm0, %xmm0\n";
        }
        out << " leaq print_float_fmt(%rip), %rdi\n";
        out << " movl $1, %eax\n"; 
        out << " call printf@PLT\n";
    } else {
	    out << " movq %rax, %rsi\n";
	    out << " leaq print_fmt(%rip), %rdi\n";
	    out << " movl $0, %eax\n";
	    out << " call printf@PLT\n";
    }
	return 0;
}

int GenCodeVisitor::visit(AssignStm* assignStmt) {
	if (assignStmt->id == "_") {
		if (assignStmt->e) {
			assignStmt->e->accept(this);
		}
		return 0;
	}

	if (!assignStmt->e) {
		throw std::runtime_error("Asignación sin expresión para " + assignStmt->id);
	}

	assignStmt->e->accept(this);

	if (auto* info = lookupSymbol(assignStmt->id)) {
		info->initialized = true;
		out << " movq %rax, " << info->offset << "(%rbp)\n";
		return 0;
	}

	auto globalIt = globalSymbols.find(assignStmt->id);
	if (globalIt != globalSymbols.end()) {
		out << " movq %rax, " << globalIt->second << "(%rip)\n";
		return 0;
	}

	throw std::runtime_error("Identificador no declarado: " + assignStmt->id);
}

int GenCodeVisitor::visit(ReturnStm* returnStmt) {
	if (returnStmt->e) {
		returnStmt->e->accept(this);
	} else {
		out << " movq $0, %rax\n";
	}
	out << " jmp " << currentReturnLabel << "\n";
	return 0;
}

int GenCodeVisitor::visit(VarDec* varDec) {
	if (!insideFunction) {
		for (const auto& name : varDec->variables) {
			if (globalSymbols.find(name) == globalSymbols.end()) {
				globalSymbols.emplace(name, name);
			}
		}
		return 0;
	}

	for (const auto& name : varDec->variables) {
		SymbolInfo tmpl;
		tmpl.isMutable = true;
		tmpl.initialized = false;
		tmpl.type = resolve_type(varDec->tipo);
		declareLocal(name, tmpl);
	}
	return 0;
}

int GenCodeVisitor::visit(BinaryExp* exp) {
	if (exp->op == ASSIGN_OP) {
        if (IdExp* idExp = dynamic_cast<IdExp*>(exp->left)) {
            string name = idExp->value;

            exp->right->accept(this);
            Type::TType rhsType = lastType;

            if (auto* info = lookupSymbol(name)) {
                if (!info->isMutable && info->initialized) {
                    // ...
                }
                info->initialized = true;
                
                string typeName = resolve_alias(info->typeName);
                int size = 8;
                if (typeName.find("[") != string::npos) {
                    size_t open = typeName.find("[");
                    size_t close = typeName.find("]");
                    int count = stoi(typeName.substr(open + 1, close - open - 1));
                    size = count * 4; 
                } else if (globalStructLayouts.count(typeName)) {
                    size = globalStructLayouts[typeName].size;
                } else if (info->type == Type::F32) {
                    size = 4;
                } else if (info->type == Type::I32 || info->type == Type::U32) {
                    size = 4;
                }

                if (info->type == Type::F32 && (rhsType == Type::F64 || rhsType == Type::NOTYPE)) {
                    // Convert double to float
                    out << " movq %rax, %xmm0\n";
                    out << " cvtsd2ss %xmm0, %xmm0\n";
                    out << " movd %xmm0, %eax\n"; // Move 32 bits to eax
                    out << " movl %eax, " << info->offset << "(%rbp)\n";
                    return 0;
                }

                if (size > 8) {
                    // %rax holds source address
                    out << " movq %rax, %rsi\n"; // Source
                    out << " leaq " << info->offset << "(%rbp), %rdi\n"; // Destination
                    out << " movq $" << size << ", %rcx\n"; // Count
                    out << " rep movsb\n";
                } else {
                    if (size == 4) {
                        out << " movl %eax, " << info->offset << "(%rbp)\n";
                    } else {
                        out << " movq %rax, " << info->offset << "(%rbp)\n";
                    }
                }
                return 0;
            }

            auto globalIt = globalSymbols.find(name);
            if (globalIt != globalSymbols.end()) {
                out << " movq %rax, " << globalIt->second << "(%rip)\n";
                return 0;
            }
            throw std::runtime_error("Identificador no declarado: " + name);

        } else if (ArrayAccessExp* arrExp = dynamic_cast<ArrayAccessExp*>(exp->left)) {
             IdExp* idArr = dynamic_cast<IdExp*>(arrExp->array);
             if (!idArr) throw std::runtime_error("Solo se soporta asignación a arrays con nombre directo");
             
             auto* info = lookupSymbol(idArr->value);
             if (!info) throw std::runtime_error("Array no declarado: " + idArr->value);
             
             string typeName = resolve_alias(info->typeName);
             int elemSize = 4;
             if (typeName.find("i64") != string::npos || typeName.find("u64") != string::npos || typeName.find("f64") != string::npos) {
                 elemSize = 8;
             }
             
             out << " leaq " << info->offset << "(%rbp), %rax\n";
             out << " pushq %rax\n"; // Save base
             
             arrExp->index->accept(this);
             out << " movq %rax, %rcx\n"; // Index in rcx
             out << " popq %rax\n"; // Base in rax
             
             out << " leaq (%rax, %rcx, " << elemSize << "), %rax\n";
             out << " pushq %rax\n"; // Save element address
             
             exp->right->accept(this);
             // Result in %rax
             
             out << " popq %rdi\n"; // Element address
             if (elemSize == 4) {
                 out << " movl %eax, (%rdi)\n";
             } else {
                 out << " movq %rax, (%rdi)\n";
             }
             
             return 0;
        } else {
            throw std::runtime_error("Lado izquierdo de asignación no es un identificador o acceso a array");
        }
	}

	if (exp->op == AND_OP) {
		string falseLabel = makeLabel("and_false");
		string endLabel = makeLabel("and_end");

		exp->left->accept(this);
		out << " cmpq $0, %rax\n";
		out << " je " << falseLabel << "\n";
		exp->right->accept(this);
		out << " cmpq $0, %rax\n";
		out << " je " << falseLabel << "\n";
		out << " movq $1, %rax\n";
		out << " jmp " << endLabel << "\n";
		out << falseLabel << ":\n";
		out << " movq $0, %rax\n";
		out << endLabel << ":\n";
		return 0;
	}

	exp->left->accept(this);
    Type::TType leftType = lastType;
	out << " pushq %rax\n";
	exp->right->accept(this);
    Type::TType rightType = lastType;
	out << " movq %rax, %rcx\n";
	out << " popq %rax\n";

    bool isFloat = (leftType == Type::F32 || leftType == Type::F64 || rightType == Type::F32 || rightType == Type::F64);

    if (isFloat) {
        out << " movq %rax, %xmm0\n";
        out << " movq %rcx, %xmm1\n";

        if (leftType == Type::F32 && rightType == Type::F32) {
             switch (exp->op) {
                case PLUS_OP: out << " addss %xmm1, %xmm0\n"; break;
                case MINUS_OP: out << " subss %xmm1, %xmm0\n"; break;
                case MUL_OP: out << " mulss %xmm1, %xmm0\n"; break;
                case DIV_OP: out << " divss %xmm1, %xmm0\n"; break;
                default: throw std::runtime_error("Float op not supported");
            }
            lastType = Type::F32;
        } else {
            if (leftType == Type::F32) out << " cvtss2sd %xmm0, %xmm0\n";
            if (rightType == Type::F32) out << " cvtss2sd %xmm1, %xmm1\n";

            switch (exp->op) {
                case PLUS_OP: out << " addsd %xmm1, %xmm0\n"; break;
                case MINUS_OP: out << " subsd %xmm1, %xmm0\n"; break;
                case MUL_OP: out << " mulsd %xmm1, %xmm0\n"; break;
                case DIV_OP: out << " divsd %xmm1, %xmm0\n"; break;
                default: throw std::runtime_error("Float op not supported");
            }
            lastType = Type::F64;
        }
        out << " movq %xmm0, %rax\n";
        return 0;
    }

	switch (exp->op) {
		case PLUS_OP:
			out << " addq %rcx, %rax\n";
			break;
		case MINUS_OP:
			out << " subq %rcx, %rax\n";
			break;
		case MUL_OP:
			out << " imulq %rcx, %rax\n";
			break;
		case DIV_OP:
			out << " cqto\n";
			out << " idivq %rcx\n";
			break;
		case LT_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " setl %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case GT_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " setg %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case LE_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " setle %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case GE_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " setge %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case EQ_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " sete %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case NEQ_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " setne %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case POW_OP:
			throw std::runtime_error("Operador potencia no soportado en generador");
		default:
			throw std::runtime_error("Operador binario no soportado");
	}
    lastType = Type::I64;
	return 0;
}

int GenCodeVisitor::visit(NumberExp* exp) {
	out << " movq $" << exp->value << ", %rax\n";
    lastType = Type::I64;
	return 0;
}

int GenCodeVisitor::visit(BoolExp* exp) {
	out << " movq $" << (exp->valor ? 1 : 0) << ", %rax\n";
	return 0;
}

int GenCodeVisitor::visit(IdExp* exp) {
	if (const auto* info = lookupSymbol(exp->value)) {
        string typeName = resolve_alias(info->typeName);
        int size = 8;
        if (typeName.find("[") != string::npos) {
             size_t open = typeName.find("[");
             size_t close = typeName.find("]");
             int count = stoi(typeName.substr(open + 1, close - open - 1));
             size = count * 4; 
        } else if (globalStructLayouts.count(typeName)) {
             size = globalStructLayouts[typeName].size;
        }

        if (size > 8) {
             out << " leaq " << info->offset << "(%rbp), %rax\n";
        } else {
             if (info->type == Type::F32 || info->type == Type::I32 || info->type == Type::U32) {
                 out << " movl " << info->offset << "(%rbp), %eax\n";
                 // Zero out high bits of rax implicitly by movl
             } else {
		         out << " movq " << info->offset << "(%rbp), %rax\n";
             }
        }
        lastType = info->type;
		return 0;
	}

	auto it = globalSymbols.find(exp->value);
	if (it != globalSymbols.end()) {
		out << " movq " << it->second << "(%rip), %rax\n";
        // Global symbols type? We don't track it well in globalSymbols map (it's just name->label).
        // Assuming I64 for now or we need a global symbol table with types.
        lastType = Type::I64; 
		return 0;
	}

	throw std::runtime_error("Identificador no declarado: " + exp->value);
}

int GenCodeVisitor::visit(FcallExp* exp) {
	vector<Exp*> args(exp->argumentos.begin(), exp->argumentos.end());
	std::size_t totalArgs = args.size();
	std::size_t stackArgs = totalArgs > kArgRegisters.size() ? totalArgs - kArgRegisters.size() : 0;

	std::size_t stackAdjust = stackArgs * 8;
	if (stackAdjust % 16 != 0) {
		out << " subq $8, %rsp\n";
		stackAdjust += 8;
	}

	for (std::size_t idx = totalArgs; idx > 0; --idx) {
		auto* arg = args[idx - 1];
		if (arg) {
			arg->accept(this);
		} else {
			out << " movq $0, %rax\n";
		}

		if (idx - 1 >= kArgRegisters.size()) {
			out << " pushq %rax\n";
		} else {
			out << " movq %rax, " << kArgRegisters[idx - 1] << "\n";
		}
	}

	out << " call " << exp->nombre << "\n";

	if (stackAdjust > 0) {
		out << " addq $" << stackAdjust << ", %rsp\n";
	}
	return 0;
}

int GenCodeVisitor::visit(StructDec* sd) {
    StructLayout layout;
    int currentOffset = 0;
    for (auto& field : sd->fields) {
        layout.offsets[field.first] = currentOffset;
        layout.types[field.first] = field.second;
        string type = field.second;
        int size = 8; 
        if (type.find("[") != string::npos) {
             size_t open = type.find("[");
             size_t close = type.find("]");
             int count = stoi(type.substr(open + 1, close - open - 1));
             size = count * 4; 
        } else if (type == "i32" || type == "bool" || type == "u32" || type == "f32") {
             size = 4;
        }
        layout.offsets[field.first] = currentOffset;
        currentOffset += size;
    }
    layout.size = currentOffset;
    globalStructLayouts[sd->name] = layout;
    return 0;
}

int GenCodeVisitor::visit(ArrayAccessExp* exp) {
    if (IdExp* id = dynamic_cast<IdExp*>(exp->array)) {
        if (const auto* info = lookupSymbol(id->value)) {
             out << " leaq " << info->offset << "(%rbp), %rax\n";
        } else {
             throw std::runtime_error("Array global no soportado");
        }
    } else {
        throw std::runtime_error("Array access only supported on identifiers");
    }
    
    out << " pushq %rax\n"; 
    exp->index->accept(this); 
    out << " movq %rax, %rcx\n"; 
    out << " popq %rax\n"; 
    
    out << " leaq (%rax, %rcx, 4), %rax\n"; 
    out << " movl (%rax), %eax\n"; 
    out << " cltq\n";
    return 0;
}

int GenCodeVisitor::visit(FieldAccessExp* exp) {
    if (IdExp* id = dynamic_cast<IdExp*>(exp->object)) {
        if (const auto* info = lookupSymbol(id->value)) {
             out << " leaq " << info->offset << "(%rbp), %rax\n";
             string typeName = resolve_alias(info->typeName);
             if (globalStructLayouts.count(typeName)) {
                 int offset = globalStructLayouts[typeName].offsets[exp->field];
                 string fieldType = globalStructLayouts[typeName].types[exp->field];
                 out << " addq $" << offset << ", %rax\n";
                 
                 if (fieldType == "i64" || fieldType == "u64" || fieldType == "f64") {
                     out << " movq (%rax), %rax\n";
                 } else {
                     out << " movl (%rax), %eax\n"; 
                     out << " cltq\n";
                 }
                 return 0;
             }
        }
    }
    throw std::runtime_error("Field access error");
}

int GenCodeVisitor::visit(StructInitExp* exp) {
    string resolvedName = resolve_alias(exp->name);
    if (globalStructLayouts.count(resolvedName)) {
        auto& layout = globalStructLayouts[resolvedName];
        int size = layout.size;

        // Allocate temporary space on stack
        int alignedSize = (size + 7) / 8 * 8;
        
        int structBaseOffset = nextStackOffset - alignedSize + 8;
        nextStackOffset -= alignedSize;

        for (auto& field : exp->fields) {
            string fname = field.first;
            Exp* expr = field.second;
            int fieldOffset = layout.offsets[fname];
            string ftype = layout.types[fname];

            expr->accept(this); // Value in %rax

            if (ftype == "i32" || ftype == "bool") {
                out << " movl %eax, " << (structBaseOffset + fieldOffset) << "(%rbp)\n";
            } else {
                out << " movq %rax, " << (structBaseOffset + fieldOffset) << "(%rbp)\n";
            }
        }

        if (size <= 8) {
            out << " movq " << structBaseOffset << "(%rbp), %rax\n";
        } else {
            out << " leaq " << structBaseOffset << "(%rbp), %rax\n";
        }
    }
    return 0;
}

int GenCodeVisitor::visit(TypeAlias* ta) {
    globalTypeAliases[ta->alias] = ta->type;
    return 0;
}

int GenCodeVisitor::visit(FloatExp* exp) {
    double v = exp->value;
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    out << " movabsq $" << bits << ", %rax\n";
    out << " movq %rax, %xmm0\n";
    lastType = exp->isDouble ? Type::F64 : Type::F32;
    return 0;
}

