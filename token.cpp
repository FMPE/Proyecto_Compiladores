#include <iostream>
#include "token.h"

using namespace std;

// -----------------------------
// Constructores
// -----------------------------

Token::Token(Type type) 
    : type(type), text("") { }

Token::Token(Type type, char c) 
    : type(type), text(string(1, c)) { }

Token::Token(Type type, const string& source, int first, int last) 
    : type(type), text(source.substr(first, last)) { }

// -----------------------------
// Sobrecarga de operador <<
// -----------------------------

// Para Token por referencia
ostream& operator<<(ostream& outs, const Token& tok) {
    switch (tok.type) {
        // Básicos
        case Token::NUMBER: outs << "TOKEN(NUMBER, \"" << tok.text << "\")"; break;
        case Token::STRING_LITERAL: outs << "TOKEN(STRING, \"" << tok.text << "\")"; break;
        case Token::IDENTIFIER: outs << "TOKEN(IDENTIFIER, \"" << tok.text << "\")"; break;
        case Token::ERR: outs << "TOKEN(ERR, \"" << tok.text << "\")"; break;
        case Token::END: outs << "TOKEN(END)"; break;

        // Puntuación
        case Token::LPAREN: outs << "TOKEN(LPAREN, \"" << tok.text << "\")"; break;
        case Token::RPAREN: outs << "TOKEN(RPAREN, \"" << tok.text << "\")"; break;
        case Token::LBRACE: outs << "TOKEN(LBRACE, \"" << tok.text << "\")"; break;
        case Token::RBRACE: outs << "TOKEN(RBRACE, \"" << tok.text << "\")"; break;
        case Token::LBRACKET: outs << "TOKEN(LBRACKET, \"" << tok.text << "\")"; break;
        case Token::RBRACKET: outs << "TOKEN(RBRACKET, \"" << tok.text << "\")"; break;
        case Token::COMMA: outs << "TOKEN(COMMA, \"" << tok.text << "\")"; break;
        case Token::SEMICOL: outs << "TOKEN(SEMICOL, \"" << tok.text << "\")"; break;
        case Token::COLON: outs << "TOKEN(COLON, \"" << tok.text << "\")"; break;
        case Token::DOT: outs << "TOKEN(DOT, \"" << tok.text << "\")"; break;
        case Token::DOTDOT: outs << "TOKEN(DOTDOT, \"" << tok.text << "\")"; break;
        case Token::ARROW: outs << "TOKEN(ARROW, \"" << tok.text << "\")"; break;

        // Operadores
        case Token::PLUS: outs << "TOKEN(PLUS, \"" << tok.text << "\")"; break;
        case Token::MINUS: outs << "TOKEN(MINUS, \"" << tok.text << "\")"; break;
        case Token::MUL: outs << "TOKEN(MUL, \"" << tok.text << "\")"; break;
        case Token::DIV: outs << "TOKEN(DIV, \"" << tok.text << "\")"; break;
        case Token::MOD: outs << "TOKEN(MOD, \"" << tok.text << "\")"; break;
        case Token::POW: outs << "TOKEN(POW, \"" << tok.text << "\")"; break;
        case Token::ASSIGN: outs << "TOKEN(ASSIGN, \"" << tok.text << "\")"; break;
        case Token::PLUS_ASSIGN: outs << "TOKEN(PLUS_ASSIGN, \"" << tok.text << "\")"; break;
        case Token::MINUS_ASSIGN: outs << "TOKEN(MINUS_ASSIGN, \"" << tok.text << "\")"; break;
        case Token::OR: outs << "TOKEN(OR, \"" << tok.text << "\")"; break;
        case Token::AND: outs << "TOKEN(AND, \"" << tok.text << "\")"; break;
        case Token::NOT: outs << "TOKEN(NOT, \"" << tok.text << "\")"; break;

        // Relacionales
        case Token::EQ: outs << "TOKEN(EQ, \"" << tok.text << "\")"; break;
        case Token::NEQ: outs << "TOKEN(NEQ, \"" << tok.text << "\")"; break;
        case Token::LT: outs << "TOKEN(LT, \"" << tok.text << "\")"; break;
        case Token::GT: outs << "TOKEN(GT, \"" << tok.text << "\")"; break;
        case Token::LE: outs << "TOKEN(LE, \"" << tok.text << "\")"; break;
        case Token::GE: outs << "TOKEN(GE, \"" << tok.text << "\")"; break;

        // Palabras clave
        case Token::FN: outs << "TOKEN(FN, \"" << tok.text << "\")"; break;
        case Token::STRUCT: outs << "TOKEN(STRUCT, \"" << tok.text << "\")"; break;
        case Token::TYPE: outs << "TOKEN(TYPE, \"" << tok.text << "\")"; break;
        case Token::LET: outs << "TOKEN(LET, \"" << tok.text << "\")"; break;
        case Token::MUT: outs << "TOKEN(MUT, \"" << tok.text << "\")"; break;
        case Token::FOR: outs << "TOKEN(FOR, \"" << tok.text << "\")"; break;
        case Token::IN: outs << "TOKEN(IN, \"" << tok.text << "\")"; break;
        case Token::IF: outs << "TOKEN(IF, \"" << tok.text << "\")"; break;
        case Token::ELSE: outs << "TOKEN(ELSE, \"" << tok.text << "\")"; break;
        case Token::WHILE: outs << "TOKEN(WHILE, \"" << tok.text << "\")"; break;
        case Token::RETURN: outs << "TOKEN(RETURN, \"" << tok.text << "\")"; break;
        case Token::PRINTLN: outs << "TOKEN(PRINTLN, \"" << tok.text << "\")"; break;

        // Tipos primitivos
        case Token::U8: outs << "TOKEN(U8)"; break;
        case Token::U16: outs << "TOKEN(U16)"; break;
        case Token::U32: outs << "TOKEN(U32)"; break;
        case Token::U64: outs << "TOKEN(U64)"; break;
        case Token::USIZE: outs << "TOKEN(USIZE)"; break;
        case Token::I32: outs << "TOKEN(I32)"; break;
        case Token::I64: outs << "TOKEN(I64)"; break;
        case Token::F32: outs << "TOKEN(F32)"; break;
        case Token::F64: outs << "TOKEN(F64)"; break;
        case Token::BOOL: outs << "TOKEN(BOOL)"; break;

        // Legacy / compatibilidad
        case Token::FUN: outs << "TOKEN(FUN, \"" << tok.text << "\")"; break;
        case Token::ENDFUN: outs << "TOKEN(ENDFUN, \"" << tok.text << "\")"; break;
        case Token::VAR: outs << "TOKEN(VAR, \"" << tok.text << "\")"; break;
        case Token::PRINT: outs << "TOKEN(PRINT, \"" << tok.text << "\")"; break;
        case Token::TRUE: outs << "TOKEN(TRUE, \"" << tok.text << "\")"; break;
        case Token::FALSE: outs << "TOKEN(FALSE, \"" << tok.text << "\")"; break;
        case Token::AND_LEGACY: outs << "TOKEN(AND_LEGACY)"; break;

        default: outs << "TOKEN(UNKNOWN, \"" << tok.text << "\")"; break;
    }
    return outs;
}

// Para Token puntero
ostream& operator<<(ostream& outs, const Token* tok) {
    if (!tok) return outs << "TOKEN(NULL)";
    return outs << *tok;  // delega al otro
}