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

// Operadores binarios soportados
enum BinaryOp { 
    PLUS_OP, 
    MINUS_OP, 
    MUL_OP, 
    DIV_OP,
    POW_OP,
    LT_OP,
    GT_OP,
    LE_OP,
    GE_OP,
    EQ_OP,
    NEQ_OP,
    AND_OP,
    ASSIGN_OP // Added for assignment expressions
};

// ============================================================
// Clase abstracta Exp
// ============================================================
class Exp {
public:
    virtual int  accept(Visitor* visitor) = 0;
    virtual ~Exp() = 0;
    static string binopToChar(BinaryOp op);
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
};

// ============================================================
// Expresión numérica
// ============================================================
class NumberExp : public Exp {
public:
    long long value;

    NumberExp(long long v);
    ~NumberExp();

    int accept(Visitor* visitor);
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
};


// ============================================================
// Clase base para sentencias
// ============================================================
class Stm {
public:
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;
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
};

// While
class WhileStm : public Stm {
public:
    Exp* condition;
    BlockStm* body;

    WhileStm(Exp* cond, BlockStm* b) : condition(cond), body(b) {}
    ~WhileStm() {}

    int accept(Visitor* visitor);
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
};
class AssignStm : public Stm {
public:
    string id;
    Exp* e;

    AssignStm(string, Exp*);
    ~AssignStm();

    int accept(Visitor* visitor);
};

class PrintStm : public Stm {
public:
    Exp* e;

    PrintStm(Exp*);
    ~PrintStm();

    int accept(Visitor* visitor);
};

class ReturnStm : public Stm {
public:
    Exp* e;

    ReturnStm() {};
    ~ReturnStm() {};

    int accept(Visitor* visitor);
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
};

class Body {
public:
    list<Stm*> stmlist;
    list<VarDec*> vdlist;

    Body() {};
    ~Body() {};
    int accept(Visitor* visitor);
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
};

class StructDec {
public:
    string name;
    vector<pair<string, string>> fields; // name, type

    StructDec(string n) : name(n) {}
    ~StructDec() {}
    int accept(Visitor* visitor);
};

class TypeAlias {
public:
    string alias;
    string type;

    TypeAlias(string a, string t) : alias(a), type(t) {}
    ~TypeAlias() {}
    int accept(Visitor* visitor);
};

class Program {
public:
    list<FunDec*> fdlist;
    list<VarDec*> vdlist;
    list<StructDec*> sdlist; // Added struct list
    list<TypeAlias*> talist; // Added type alias list

    Program();
    ~Program();
    int accept(Visitor* visitor);

};

// ============================================================
// Acceso a arreglo
// ============================================================
class ArrayAccessExp : public Exp {
public:
    Exp* array;
    Exp* index;

    ArrayAccessExp(Exp* a, Exp* i);
    ~ArrayAccessExp();

    int accept(Visitor* visitor);
};

// ============================================================
// Acceso a campo de struct
// ============================================================
class FieldAccessExp : public Exp {
public:
    Exp* object;
    string field;

    FieldAccessExp(Exp* o, string f);
    ~FieldAccessExp();

    int accept(Visitor* visitor);
};

// ============================================================
// Inicialización de struct
// ============================================================
class StructInitExp : public Exp {
public:
    string name;
    vector<pair<string, Exp*>> fields;

    StructInitExp(string n) : name(n) {}
    ~StructInitExp() { for(auto f : fields) delete f.second; }

    int accept(Visitor* visitor);
};

// ============================================================
// Expresión flotante
// ============================================================
class FloatExp : public Exp {
public:
    double value;
    bool isDouble; // true for f64, false for f32

    FloatExp(double v, bool d);
    ~FloatExp();

    int accept(Visitor* visitor);
};

#endif // AST_H
