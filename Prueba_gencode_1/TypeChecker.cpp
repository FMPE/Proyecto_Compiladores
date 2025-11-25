#include "typechecker.h"
#include <iostream>
#include <stdexcept>
using namespace std;


Type* NumberExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* IdExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* BinaryExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FcallExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* BoolExp::accept(TypeVisitor* v) { return v->visit(this); }

void AssignStm::accept(TypeVisitor* v) { v->visit(this); }
void PrintStm::accept(TypeVisitor* v) { v->visit(this); }
void ReturnStm::accept(TypeVisitor* v) { v->visit(this); }

void VarDec::accept(TypeVisitor* v) { v->visit(this); }
void FunDec::accept(TypeVisitor* v) { v->visit(this); }
void Body::accept(TypeVisitor* v) { v->visit(this); }
void Program::accept(TypeVisitor* v) { v->visit(this); }

// ===========================================================
//   Constructor del TypeChecker
// ===========================================================

TypeChecker::TypeChecker() {
    // Usamos i32 como entero por defecto
    intType = new Type(Type::I32);
    boolType = new Type(Type::BOOL);
    voidType = new Type(Type::VOID);
}

// ===========================================================
//   Registrar funciones globales
// ===========================================================

void TypeChecker::add_function(FunDec* fd) {
    if (functions.find(fd->nombre) != functions.end()) {
        cerr << "Error: función '" << fd->nombre << "' ya fue declarada." << endl;
        exit(0);
    }

    Type* returnType = new Type();
    if (!returnType->set_basic_type(fd->tipo)) {
        cerr << "Error: tipo de retorno no válido en función '" << fd->nombre << "' (" << fd->tipo << ")." << endl;
        exit(0);
    }

    functions[fd->nombre] = returnType;
}

// ===========================================================
//   Método principal de verificación
// ===========================================================

void TypeChecker::typecheck(Program* program) {
    if (program) program->accept(this);
    cout << "Revisión exitosa" << endl;
}

// ===========================================================
//   Nivel superior: Programa y Bloque
// ===========================================================

void TypeChecker::visit(Program* p) {
    // Primero registrar funciones
    for (auto f : p->fdlist)
        add_function(f);

    env.push_scope();
    for (auto v : p->vdlist)
        v->accept(this);  
    for (auto f : p->fdlist)
        f->accept(this);  
    env.pop_scope();
}

void TypeChecker::visit(Body* b) {
    env.push_scope();
    for (auto v : b->vdlist)
        v->accept(this); 
    for (auto s : b->stmlist)
        s->accept(this); 
    env.pop_scope();
}

// ===========================================================
//   Declaraciones
// ===========================================================

void TypeChecker::visit(VarDec* v) {
    Type* t = new Type();
    if (!t->set_basic_type(v->tipo)) {
        cerr << "Error: tipo de variable no válido." << endl;
        exit(0);
    }

    for (const auto& id : v->variables) {
        if (env.contains(id)) {
            cerr << "Error: variable '" << id << "' ya declarada." << endl;
            exit(0);
        }
        env.declare(id, t);
    }
}

void TypeChecker::visit(FunDec* f) {
    env.push_scope();
    for (size_t i = 0; i < f->Nparametros.size(); ++i) {
        Type* pt = new Type();
        if (!pt->set_basic_type(f->Tparametros[i])) {
            cerr << "Error: tipo de parámetro inválido ('" << f->Tparametros[i] << "') en función '" << f->nombre << "'." << endl;
            exit(0);
        }
        env.declare(f->Nparametros[i], pt);
    }
    f->cuerpo->accept(this); 
    env.pop_scope();
}

// ===========================================================
//   Sentencias
// ===========================================================

void TypeChecker::visit(PrintStm* stm) {
    Type* t = stm->e->accept(this);
    if (!(t->match(intType) || t->match(boolType))) {
        cerr << "Error: tipo inválido en print (solo int o bool)." << endl;
        exit(0);
    }
}

void TypeChecker::visit(AssignStm* stm) {
    if (!env.contains(stm->id)) {
        cerr << "Error: variable '" << stm->id << "' no declarada." << endl;
        exit(0);
    }

    Type** entry = env.lookup(stm->id);
    if (!entry) {
        cerr << "Error interno: entrada de variable perdida." << endl;
        exit(0);
    }
    Type* varType = *entry;
    Type* expType = stm->e->accept(this);

    if (!varType->match(expType)) {
        cerr << "Error: tipos incompatibles en asignación a '" << stm->id << "'." << endl;
        exit(0);
    }
}

void TypeChecker::visit(ReturnStm* stm) {
    if (stm->e) {
        Type* t = stm->e->accept(this);
        // Permitimos cualquier tipo numérico o bool o void (aunque void con expresión no tendría sentido, se ignora la verificación más estricta)
        if (!(Type::is_numeric(t->ttype) || t->match(boolType) || t->match(voidType))) {
            cerr << "Error: tipo inválido en return." << endl;
            exit(0);
        }
    }
}

// ===========================================================
//   Expresiones
// ===========================================================

Type* TypeChecker::visit(BinaryExp* e) {
    Type* left = e->left->accept(this);
    Type* right = e->right->accept(this);

    switch (e->op) {
        case PLUS_OP: 
        case MINUS_OP: 
        case MUL_OP: 
        case DIV_OP: 
        case POW_OP:
            if (!(Type::is_numeric(left->ttype) && Type::is_numeric(right->ttype) && left->ttype == right->ttype)) {
                cerr << "Error: operación aritmética requiere operandos numéricos del mismo tipo." << endl;
                exit(0);
            }
            return left; // mismo tipo
        case LE_OP:
            if (!(Type::is_numeric(left->ttype) && Type::is_numeric(right->ttype) && left->ttype == right->ttype)) {
                cerr << "Error: comparación requiere operandos numéricos del mismo tipo." << endl;
                exit(0);
            }
            return boolType;
        case AND_OP:
            if (!(left->match(boolType) && right->match(boolType))) {
                cerr << "Error: operación lógica requiere operandos bool." << endl;
                exit(0);
            }
            return boolType;
        default:
            cerr << "Error: operador binario no soportado." << endl;
            exit(0);
    }
}

Type* TypeChecker::visit(NumberExp* e) { return intType; }

Type* TypeChecker::visit(BoolExp* e) { return boolType; }

Type* TypeChecker::visit(IdExp* e) {
    if (!env.contains(e->value)) {
        cerr << "Error: variable '" << e->value << "' no declarada." << endl;
        exit(0);
    }
    Type** entry = env.lookup(e->value);
    if (!entry) {
        cerr << "Error interno: entrada de variable perdida." << endl;
        exit(0);
    }
    return *entry;
}

Type* TypeChecker::visit(FcallExp* e) {
    auto it = functions.find(e->nombre);
    if (it == functions.end()) {
        cerr << "Error: llamada a función no declarada '" << e->nombre << "'." << endl;
        exit(0);
    }
    return it->second;
}
