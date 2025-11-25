#include "visitor.h"

#include "ast.h"

#include <stdexcept>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace {
const vector<string> kArgRegisters = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

Type::TType resolve_type(const string& name) {
	auto tt = Type::string_to_type(name);
	return tt;
}
}

// -----------------------------------------------------------------------------
// Accept implementations (kept alongside visitor logic for clarity)
// -----------------------------------------------------------------------------

int BinaryExp::accept(Visitor* visitor) { return visitor->visit(this); }
int NumberExp::accept(Visitor* visitor) { return visitor->visit(this); }
int BoolExp::accept(Visitor* visitor) { return visitor->visit(this); }
int IdExp::accept(Visitor* visitor) { return visitor->visit(this); }
int FcallExp::accept(Visitor* visitor) { return visitor->visit(this); }

int Program::accept(Visitor* visitor) { return visitor->visit(this); }
int FunDec::accept(Visitor* visitor) { return visitor->visit(this); }
int Body::accept(Visitor* visitor) { return visitor->visit(this); }
int VarDec::accept(Visitor* visitor) { return visitor->visit(this); }
int BlockStm::accept(Visitor* visitor) { return visitor->visit(this); }
int LetStm::accept(Visitor* visitor) { return visitor->visit(this); }
int PrintStm::accept(Visitor* visitor) { return visitor->visit(this); }
int AssignStm::accept(Visitor* visitor) { return visitor->visit(this); }
int IfStm::accept(Visitor* visitor) { return visitor->visit(this); }
int WhileStm::accept(Visitor* visitor) { return visitor->visit(this); }
int ForStm::accept(Visitor* visitor) { return visitor->visit(this); }
int ReturnStm::accept(Visitor* visitor) { return visitor->visit(this); }

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
	++currentSlotCount;
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

int TypeCheckerVisitor::visit(FcallExp* exp) {
	for (auto arg : exp->argumentos) {
		if (arg) {
			arg->accept(this);
		}
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

	for (auto globalDecl : program->vdlist) {
		if (globalDecl) {
			globalDecl->accept(this);
		}
	}

	for (auto it = globalSymbols.begin(); it != globalSymbols.end(); ++it) {
		out << it->second << ": .quad 0\n";
	}

	out << ".text\n";

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

	SymbolInfo info = declareLocal(letStmt->name, tmpl);

	if (letStmt->init) {
		letStmt->init->accept(this);
		out << " movq %rax, " << info.offset << "(%rbp)\n";
	} else {
		out << " movq $0, " << info.offset << "(%rbp)\n";
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

	out << " movq %rax, %rsi\n";
	out << " leaq print_fmt(%rip), %rdi\n";
	out << " movl $0, %eax\n";
	out << " call printf@PLT\n";
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
	out << " pushq %rax\n";
	exp->right->accept(this);
	out << " movq %rax, %rcx\n";
	out << " popq %rax\n";

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
		case LE_OP:
			out << " cmpq %rcx, %rax\n";
			out << " movq $0, %rax\n";
			out << " setle %al\n";
			out << " movzbq %al, %rax\n";
			break;
		case POW_OP:
			throw std::runtime_error("Operador potencia no soportado en generador");
		default:
			throw std::runtime_error("Operador binario no soportado");
	}

	return 0;
}

int GenCodeVisitor::visit(NumberExp* exp) {
	out << " movq $" << exp->value << ", %rax\n";
	return 0;
}

int GenCodeVisitor::visit(BoolExp* exp) {
	out << " movq $" << (exp->valor ? 1 : 0) << ", %rax\n";
	return 0;
}

int GenCodeVisitor::visit(IdExp* exp) {
	if (const auto* info = lookupSymbol(exp->value)) {
		out << " movq " << info->offset << "(%rbp), %rax\n";
		return 0;
	}

	auto it = globalSymbols.find(exp->value);
	if (it != globalSymbols.end()) {
		out << " movq " << it->second << "(%rip), %rax\n";
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

