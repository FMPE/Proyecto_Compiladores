#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>

using namespace std;

class Token {
public:
    // Tipos de token
    enum Type {
        // Literales y básicos
        NUMBER,          // Número entero (antes NUM)
        NUM = NUMBER,    // Alias legacy para compatibilidad
        STRING_LITERAL,  // "..."
        IDENTIFIER,      // Identificador
        ID = IDENTIFIER, // Alias legacy
        ERR,             // Error léxico
        END,             // Fin de entrada

        // Puntuación
        LPAREN, RPAREN,          // ( )
        LBRACE, RBRACE,          // { }
        LBRACKET, RBRACKET,      // [ ]
        COMMA,                   // ,
        COMA = COMMA,            // alias legacy
        SEMICOL,                 // ;
        COLON,                   // :
        DOT,                     // .
        DOTDOT,                  // ..
        ARROW,                   // ->

        // Operadores aritméticos y lógicos
        PLUS, MINUS, MUL, DIV, MOD, POW,      // + - * / % **
        ASSIGN,                  // =
        PLUS_ASSIGN, MINUS_ASSIGN, // += -=
        OR, AND,                 // || &&
        NOT,                     // !

        // Operadores relacionales / igualdad
        EQ, NEQ, LT, GT, LE, GE, // == != < > <= >=

        // Palabras clave
        FN, STRUCT, TYPE, LET, MUT, FOR, IN, IF, ELSE, WHILE, RETURN, PRINTLN,

        // Tipos primitivos
        U8, U16, U32, U64, USIZE, I32, I64, F32, F64, BOOL,

        // Otros (compatibilidad con código previo)
        TRUE, FALSE, FUN, ENDFUN, VAR, PRINT, // algunos legacy para mantener visitantes/AST temporalmente
        AND_LEGACY // marcador para distinguir si hace falta
    };

    // Atributos
    Type type;
    string text;

    // Constructores
    Token(Type type);
    Token(Type type, char c);
    Token(Type type, const string& source, int first, int last);

    // Sobrecarga de operadores de salida
    friend ostream& operator<<(ostream& outs, const Token& tok);
    friend ostream& operator<<(ostream& outs, const Token* tok);
};

#endif // TOKEN_H