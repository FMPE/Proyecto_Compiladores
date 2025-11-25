#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <iostream>
#include <string>
using namespace std;

// ===========================================================
//  Representación de tipos básicos del lenguaje
// ===========================================================

class Type {
public:
    // Ampliamos el conjunto para reflejar tokens Rust simplificados
    enum TType { NOTYPE, VOID, BOOL, I32, I64, U32, U64, F32, F64 };
    static const char* type_names[9];

    TType ttype;

    Type() : ttype(NOTYPE) {}
    Type(TType tt) : ttype(tt) {}

    bool match(Type* t) const { return this->ttype == t->ttype; }

    bool set_basic_type(const string& s) {
        TType tt = string_to_type(s);
        if (tt == NOTYPE) return false;
        ttype = tt;
        return true;
    }

    static bool is_numeric(TType tt) {
        return tt == I32 || tt == I64 || tt == U32 || tt == U64 || tt == F32 || tt == F64;
    }

    static TType string_to_type(const string& s) {
        if (s == "void") return VOID;
        if (s == "bool") return BOOL;
        if (s == "int" || s == "i32") return I32; // permitimos "int" legacy
        if (s == "i64") return I64;
        if (s == "u32") return U32;
        if (s == "u64") return U64;
        if (s == "f32") return F32;
        if (s == "f64") return F64;
        return NOTYPE;
    }
};

inline const char* Type::type_names[9] = { "notype", "void", "bool", "i32", "i64", "u32", "u64", "f32", "f64" };

#endif // SEMANTIC_TYPES_H
