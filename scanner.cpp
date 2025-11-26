#include <iostream>
#include <cstring>
#include <fstream>
#include "token.h"
#include "scanner.h"

using namespace std;

// -----------------------------
// Constructor
// -----------------------------
Scanner::Scanner(const char* s): input(s), first(0), current(0) { 
    }

// -----------------------------
// Función auxiliar
// -----------------------------

bool is_white_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

// -----------------------------
// nextToken: obtiene el siguiente token
// -----------------------------


Token* Scanner::nextToken() {
    auto peek = [&](int offset = 0) -> char {
        int idx = current + offset;
        if (idx >= (int)input.size()) return '\0';
        return input[idx];
    };

    // Saltar espacios
    while (current < (int)input.size() && is_white_space(input[current])) current++;
    if (current >= (int)input.size()) return new Token(Token::END);

    first = current;
    char c = peek();

    // Números
    if (isdigit(c)) {
        while (isdigit(peek())) current++;
        // Check for float
        if (peek() == '.' && isdigit(peek(1))) {
            current++; // consume dot
            while (isdigit(peek())) current++;
        }
        return new Token(Token::NUMBER, input, first, current - first);
    }

    // Identificadores / palabras clave / macro println!
    if (isalpha(c) || c == '_') {
        while (isalnum(peek()) || peek()=='_') current++;
        string lexema = input.substr(first, current - first);
        // Macro println!
        if (lexema == "println") {
            if (peek() == '!') { current++; return new Token(Token::PRINTLN, "println!", 0, 8); }
            return new Token(Token::PRINTLN, lexema, 0, (int)lexema.size());
        }
        // Palabras clave / tipos primitivos
        if (lexema == "fn") return new Token(Token::FN, lexema, 0, (int)lexema.size());
        if (lexema == "struct") return new Token(Token::STRUCT, lexema, 0, (int)lexema.size());
        if (lexema == "type") return new Token(Token::TYPE, lexema, 0, (int)lexema.size());
        if (lexema == "let") return new Token(Token::LET, lexema, 0, (int)lexema.size());
        if (lexema == "mut") return new Token(Token::MUT, lexema, 0, (int)lexema.size());
        if (lexema == "for") return new Token(Token::FOR, lexema, 0, (int)lexema.size());
        if (lexema == "in") return new Token(Token::IN, lexema, 0, (int)lexema.size());
        if (lexema == "if") return new Token(Token::IF, lexema, 0, (int)lexema.size());
        if (lexema == "else") return new Token(Token::ELSE, lexema, 0, (int)lexema.size());
        if (lexema == "while") return new Token(Token::WHILE, lexema, 0, (int)lexema.size());
        if (lexema == "return") return new Token(Token::RETURN, lexema, 0, (int)lexema.size());
        // Tipos primitivos
        if (lexema == "u8") return new Token(Token::U8, lexema, 0, (int)lexema.size());
        if (lexema == "u16") return new Token(Token::U16, lexema, 0, (int)lexema.size());
        if (lexema == "u32") return new Token(Token::U32, lexema, 0, (int)lexema.size());
        if (lexema == "u64") return new Token(Token::U64, lexema, 0, (int)lexema.size());
        if (lexema == "usize") return new Token(Token::USIZE, lexema, 0, (int)lexema.size());
        if (lexema == "i32") return new Token(Token::I32, lexema, 0, (int)lexema.size());
        if (lexema == "i64") return new Token(Token::I64, lexema, 0, (int)lexema.size());
        if (lexema == "f32") return new Token(Token::F32, lexema, 0, (int)lexema.size());
        if (lexema == "f64") return new Token(Token::F64, lexema, 0, (int)lexema.size());
        if (lexema == "bool") return new Token(Token::BOOL, lexema, 0, (int)lexema.size());
        if (lexema == "true") return new Token(Token::TRUE, lexema, 0, (int)lexema.size());
        if (lexema == "false") return new Token(Token::FALSE, lexema, 0, (int)lexema.size());
        // Legacy soportado
        if (lexema == "var") return new Token(Token::VAR, lexema, 0, (int)lexema.size());
        if (lexema == "fun") return new Token(Token::FUN, lexema, 0, (int)lexema.size());
        if (lexema == "endfun") return new Token(Token::ENDFUN, lexema, 0, (int)lexema.size());
        if (lexema == "print") return new Token(Token::PRINT, lexema, 0, (int)lexema.size());
        if (lexema == "and") return new Token(Token::AND, lexema, 0, (int)lexema.size());
        return new Token(Token::IDENTIFIER, input, first, current - first);
    }

    // String literal
    if (c == '"') {
        current++; // skip opening
        while (peek() != '"' && peek() != '\0') {
            if (peek() == '\n') break; // no multi-line strings
            current++;
        }
        if (peek() == '"') {
            current++; // consume closing quote
            return new Token(Token::STRING_LITERAL, input, first, current - first);
        }
        // error: string no cerrada
        return new Token(Token::ERR, '"');
    }

    // Multi-character operadores y símbolos
    // Orden importa (más largos primero)
    // ..
    if (c == '.' && peek(1) == '.') {
        current += 2;
        return new Token(Token::DOTDOT, "..", 0, 2);
    }
    if (c == '-' && peek(1) == '>') { current += 2; return new Token(Token::ARROW, "->", 0, 2); }
    if (c == '+' && peek(1) == '=') { current += 2; return new Token(Token::PLUS_ASSIGN, "+=", 0, 2); }
    if (c == '-' && peek(1) == '=') { current += 2; return new Token(Token::MINUS_ASSIGN, "-=", 0, 2); }
    if (c == '=' && peek(1) == '=') { current += 2; return new Token(Token::EQ, "==", 0, 2); }
    if (c == '!' && peek(1) == '=') { current += 2; return new Token(Token::NEQ, "!=", 0, 2); }
    if (c == '<' && peek(1) == '=') { current += 2; return new Token(Token::LE, "<=", 0, 2); }
    if (c == '>' && peek(1) == '=') { current += 2; return new Token(Token::GE, ">=", 0, 2); }
    if (c == '|' && peek(1) == '|') { current += 2; return new Token(Token::OR, "||", 0, 2); }
    if (c == '&' && peek(1) == '&') { current += 2; return new Token(Token::AND, "&&", 0, 2); }
    if (c == '*' && peek(1) == '*') { current += 2; return new Token(Token::POW, "**", 0, 2); }

    // Un solo caracter
    current++;
    switch (c) {
        case '+': return new Token(Token::PLUS, "+", 0, 1);
        case '-': return new Token(Token::MINUS, "-", 0, 1);
        case '*': return new Token(Token::MUL, "*", 0, 1);
        case '/': return new Token(Token::DIV, "/", 0, 1);
        case '%': return new Token(Token::MOD, "%", 0, 1);
        case '(': return new Token(Token::LPAREN, "(", 0, 1);
        case ')': return new Token(Token::RPAREN, ")", 0, 1);
        case '{': return new Token(Token::LBRACE, "{", 0, 1);
        case '}': return new Token(Token::RBRACE, "}", 0, 1);
        case '[': return new Token(Token::LBRACKET, "[", 0, 1);
        case ']': return new Token(Token::RBRACKET, "]", 0, 1);
        case ',': return new Token(Token::COMMA, ",", 0, 1);
        case ';': return new Token(Token::SEMICOL, ";", 0, 1);
        case ':': return new Token(Token::COLON, ":", 0, 1);
        case '.': return new Token(Token::DOT, ".", 0, 1);
        case '=': return new Token(Token::ASSIGN, "=", 0, 1);
        case '<': return new Token(Token::LT, "<", 0, 1);
        case '>': return new Token(Token::GT, ">", 0, 1);
        case '!': return new Token(Token::NOT, "!", 0, 1);
        default: return new Token(Token::ERR, c);
    }
}




// -----------------------------
// Destructor
// -----------------------------
Scanner::~Scanner() { }

// -----------------------------
// Función de prueba
// -----------------------------

void ejecutar_scanner(Scanner* scanner, const string& InputFile) {
    Token* tok;

    // Crear nombre para archivo de salida
    string OutputFileName = InputFile;
    size_t pos = OutputFileName.find_last_of(".");
    if (pos != string::npos) {
        OutputFileName = OutputFileName.substr(0, pos);
    }
    OutputFileName += "_tokens.txt";

    ofstream outFile(OutputFileName);
    if (!outFile.is_open()) {
        cerr << "Error: no se pudo abrir el archivo " << OutputFileName << endl;
        return;
    }

    outFile << "Scanner\n" << endl;

    while (true) {
        tok = scanner->nextToken();

        if (tok->type == Token::END) {
            outFile << *tok << endl;
            delete tok;
            outFile << "\nScanner exitoso" << endl << endl;
            outFile.close();
            return;
        }

        if (tok->type == Token::ERR) {
            outFile << *tok << endl;
            delete tok;
            outFile << "Caracter invalido" << endl << endl;
            outFile << "Scanner no exitoso" << endl << endl;
            outFile.close();
            return;
        }

        outFile << *tok << endl;
        delete tok;
    }
}
