#include "ast.h"
#include "visitor.h"
#include <iostream>

using namespace std;

// ------------------ Exp ------------------
Exp::~Exp() {}

string Exp::binopToChar(BinaryOp op) {
    switch (op) {
        case PLUS_OP:  return "+";
        case MINUS_OP: return "-";
        case MUL_OP:   return "*";
        case DIV_OP:   return "/";
        case POW_OP:   return "**";
        case LT_OP:    return "<";
        case GT_OP:    return ">";
        case LE_OP:    return "<=";
        case GE_OP:    return ">=";
        case EQ_OP:    return "==";
        case NEQ_OP:   return "!=";
        case AND_OP:   return "and";
        case ASSIGN_OP: return "=";
        default:       return "?";
    }
}

// ------------------ BinaryExp ------------------
BinaryExp::BinaryExp(Exp* l, Exp* r, BinaryOp o)
    : left(l), right(r), op(o) {}

    
BinaryExp::~BinaryExp() {
    delete left;
    delete right;
}



// ------------------ NumberExp ------------------
NumberExp::NumberExp(long long v) : value(v) {}

NumberExp::~NumberExp() {}


// ------------------idExp ------------------
IdExp::IdExp(string v) : value(v) {}

IdExp::~IdExp() {}

Stm::~Stm(){}

PrintStm::~PrintStm(){}

AssignStm::~AssignStm(){}

Program::~Program() {
    for (auto f : fdlist) delete f;
    for (auto v : vdlist) delete v;
    for (auto s : sdlist) delete s;
}

PrintStm::PrintStm(Exp* expresion){
    e=expresion;
}

AssignStm::AssignStm(string variable,Exp* expresion){
    id = variable;
    e = expresion;
}

Program::Program() {}
int Program::accept(Visitor* visitor) { return visitor->visit(this); }

// Nuevos nodos de sentencias: implementaciones mínimas

// ------------------ ArrayAccessExp ------------------
ArrayAccessExp::ArrayAccessExp(Exp* a, Exp* i) : array(a), index(i) {}
ArrayAccessExp::~ArrayAccessExp() { delete array; delete index; }
int ArrayAccessExp::accept(Visitor* visitor) { return visitor->visit(this); }

// ------------------ FieldAccessExp ------------------
FieldAccessExp::FieldAccessExp(Exp* o, string f) : object(o), field(f) {}
FieldAccessExp::~FieldAccessExp() { delete object; }
int FieldAccessExp::accept(Visitor* visitor) { return visitor->visit(this); }

// ------------------ StructDec ------------------
int StructDec::accept(Visitor* visitor) { return visitor->visit(this); }

// ------------------ TypeAlias ------------------
int TypeAlias::accept(Visitor* visitor) { return visitor->visit(this); }

// ------------------ StructInitExp ------------------
int StructInitExp::accept(Visitor* visitor) { return visitor->visit(this); }

// ------------------ Missing Accept Implementations ------------------
int BinaryExp::accept(Visitor* visitor) { return visitor->visit(this); }
int NumberExp::accept(Visitor* visitor) { return visitor->visit(this); }
int BoolExp::accept(Visitor* visitor) { return visitor->visit(this); }
int IdExp::accept(Visitor* visitor) { return visitor->visit(this); }
int FloatExp::accept(Visitor* visitor) { return visitor->visit(this); }
int FcallExp::accept(Visitor* visitor) { return visitor->visit(this); }

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

// ------------------ FloatExp ------------------
FloatExp::FloatExp(double v, bool d) : value(v), isDouble(d) {}

FloatExp::~FloatExp() {}
