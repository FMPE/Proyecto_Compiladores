#ifndef AST_H
#define AST_H

#include <string>
#include <unordered_map>
#include <list>
#include <ostream>
#include <vector>
#include "semantic_types.h"

using namespace std;

class Visitor;
class TypeVisitor; // 👈 nuevo forward declaration

// Operadores binarios soportados
enum BinaryOp { 
    PLUS_OP, 
    MINUS_OP, 
    MUL_OP, 
    DIV_OP,
    POW_OP,
    LE_OP,
    AND_OP
};

// ============================================================
// Clase abstracta Exp
// ============================================================
class Exp {
public:
    virtual int  accept(Visitor* visitor) = 0;
    virtual ~Exp() = 0;
    static string binopToChar(BinaryOp op);

    // --- NUEVO ---
    virtual Type* accept(TypeVisitor* visitor) = 0; // Para verificador de tipos
};

// ============================================================
// Expresión binaria
// ============================================================
class BinaryExp : public Exp {
public:
    Exp* left;
    Exp* right;
    BinaryOp op;

    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    ~BinaryExp();

    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor); // nuevo
};

// ============================================================
// Expresión numérica
// ============================================================
class NumberExp : public Exp {
public:
    int value;

    NumberExp(int v);
    ~NumberExp();

    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor); // nuevo
};

// ============================================================
// Expresión de identificador
// ============================================================
class IdExp : public Exp {
public:
    string value;

    IdExp(string v);
    ~IdExp();

    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor); // nuevo
};

// ============================================================
// Llamada a función
// ============================================================
class FcallExp : public Exp {
public:
    string nombre;
    list<Exp*> argumentos;

    FcallExp(){};
    ~FcallExp(){};

    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor); // nuevo
};

// ============================================================
// Booleanos
// ============================================================
class BoolExp : public Exp {
public:
    int valor;

    BoolExp(){};
    ~BoolExp(){};

    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor); // nuevo
};


// ============================================================
// Clase base para sentencias
// ============================================================
class Stm {
public:
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;

    // --- NUEVO ---
    virtual void accept(TypeVisitor* visitor) = 0;
};

// ============================================================
// Sentencias
// ============================================================
// Bloque de sentencias (equivalente a Block en la gramática Rust)
class BlockStm : public Stm {
public:
    list<Stm*> statements;

    BlockStm() {}
    ~BlockStm() {}

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
};

// Sentencia let (declaración de variable)
class LetStm : public Stm {
public:
    bool mutable_flag;
    string name;
    string type_name; // i32, bool, identificador de tipo, etc.
    Exp* init;        // puede ser null

    LetStm(bool mut, const string& n, const string& t, Exp* e)
        : mutable_flag(mut), name(n), type_name(t), init(e) {}
    ~LetStm() {}

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
};

// If / Else
class IfStm : public Stm {
public:
    Exp* condition;
    BlockStm* thenBlock;
    BlockStm* elseBlock; // puede ser null

    IfStm(Exp* cond, BlockStm* thenB, BlockStm* elseB)
        : condition(cond), thenBlock(thenB), elseBlock(elseB) {}
    ~IfStm() {}

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
};

// While
class WhileStm : public Stm {
public:
    Exp* condition;
    BlockStm* body;

    WhileStm(Exp* cond, BlockStm* b) : condition(cond), body(b) {}
    ~WhileStm() {}

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
};

// For de rango: for i in a..b { body }
class ForStm : public Stm {
public:
    string iteratorName;
    Exp* start;
    Exp* end;
    BlockStm* body;

    ForStm(const string& it, Exp* s, Exp* e, BlockStm* b)
        : iteratorName(it), start(s), end(e), body(b) {}
    ~ForStm() {}

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
};
class AssignStm : public Stm {
public:
    string id;
    Exp* e;

    AssignStm(string, Exp*);
    ~AssignStm();

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

class PrintStm : public Stm {
public:
    Exp* e;

    PrintStm(Exp*);
    ~PrintStm();

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

class ReturnStm : public Stm {
public:
    Exp* e;

    ReturnStm() {};
    ~ReturnStm() {};

    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

// ============================================================
// Declaraciones, cuerpos y programa
// ============================================================
class VarDec {
public:
    string tipo;
    list<string> variables;

    VarDec() {};
    ~VarDec() {};
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

class Body {
public:
    list<Stm*> stmlist;
    list<VarDec*> vdlist;

    Body() {};
    ~Body() {};
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

class FunDec {
public:
    string tipo;
    string nombre;
    vector<string> Tparametros;
    vector<string> Nparametros;
    Body* cuerpo;

    FunDec() {};
    ~FunDec() {};
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

class Program {
public:
    list<FunDec*> fdlist;
    list<VarDec*> vdlist;

    Program();
    ~Program();
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor); // nuevo
};

#endif // AST_H
