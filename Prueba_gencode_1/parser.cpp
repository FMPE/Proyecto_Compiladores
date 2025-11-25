// Nuevo parser para gramática Rust simplificada (consumo + AST mínima)
#include <iostream>
#include <stdexcept>
#include "token.h"
#include "scanner.h"
#include "ast.h"
#include "parser.h"

using namespace std;

Parser::Parser(Scanner* sc): scanner(sc), current(nullptr), previous(nullptr) {
    advance();
}

bool Parser::check(Token::Type t){
    if (isAtEnd()) return false;
    return current->type == t;
}

void Parser::advance(){
    if (previous) delete previous;
    previous = current;
    current = scanner->nextToken();
    if (current->type == Token::ERR){
        throw runtime_error("Error léxico: token inválido '" + current->text + "'");
    }
}

bool Parser::match(Token::Type t){
    if (check(t)){ advance(); return true; }
    return false;
}

bool Parser::isAtEnd(){ return current && current->type == Token::END; }

void Parser::consume(Token::Type t, const string& msg){
    if (!match(t)) throw runtime_error("Error sintáctico: se esperaba " + msg);
}

Program* Parser::parseProgram(){
    Program* p = new Program();
    parseItems(p);
    if (!isAtEnd()) throw runtime_error("Error sintáctico: tokens restantes tras parseo");
    cout << "Parseo exitoso" << endl;
    return p;
}

void Parser::parseItems(Program* p){
    while(!isAtEnd()){
        // Mirar posibles comienzos de Item
        if (check(Token::FN)){
            p->fdlist.push_back(parseFunction());
        } else if (check(Token::STRUCT)) {
            parseStruct();
        } else if (check(Token::TYPE)) {
            parseTypeAlias();
        } else {
            // En Rust top-level sólo se permiten estos en nuestra gramática
            break;
        }
    }
}

FunDec* Parser::parseFunction(){
    consume(Token::FN, "'fn'");
    consume(Token::IDENTIFIER, "nombre de función");
    string nombre = previous->text;
    consume(Token::LPAREN, "'('");
    // ParamListOpt
    vector<string> paramN;
    vector<string> paramT;
    if (check(Token::IDENTIFIER)){
        // Param: IDENTIFIER ':' Type
        consume(Token::IDENTIFIER, "param nombre");
        string pname = previous->text;
        consume(Token::COLON, ":");
        // Type (simplificado: consumimos primer token que puede iniciar Type)
        if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::BOOL)) {
            paramN.push_back(pname);
            paramT.push_back(previous->text);
            while(match(Token::COMMA)){
                consume(Token::IDENTIFIER, "param nombre");
                pname = previous->text;
                consume(Token::COLON, ":");
                if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::BOOL)) {
                    paramN.push_back(pname);
                    paramT.push_back(previous->text);
                } else throw runtime_error("Tipo esperado en parámetro");
            }
        } else throw runtime_error("Tipo de primer parámetro esperado");
    }
    consume(Token::RPAREN, "')'");
    // Retorno opcional -> Type
    string retType = "void";
    if (match(Token::ARROW)){
        if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::BOOL)){
            retType = previous->text;
        } else throw runtime_error("Tipo de retorno esperado tras '->'");
    }
    // Block
    BlockStm* bodyBlock = parseBlock();
    // Construir FunDec con cuerpo real
    FunDec* fd = new FunDec();
    fd->nombre = nombre;
    fd->tipo = retType;
    fd->Nparametros = paramN;
    fd->Tparametros = paramT;
    fd->cuerpo = new Body();
    fd->cuerpo->stmlist.push_back(bodyBlock);
    return fd;
}

void Parser::parseStruct(){
    consume(Token::STRUCT, "'struct'");
    consume(Token::IDENTIFIER, "nombre de struct");
    consume(Token::LBRACE, "'{' struct");
    // Campos: IDENT ':' Type ';'
    while(check(Token::IDENTIFIER)){
        advance(); // nombre campo
        consume(Token::COLON, ": en campo struct");
        // Type simplificada
        if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::BOOL)) {
            // ArrayType opcional
            if (match(Token::LBRACKET)){
                consume(Token::NUMBER, "tamaño array en campo struct");
                consume(Token::RBRACKET, "]");
            }
        } else throw runtime_error("Tipo esperado en campo struct");
        consume(Token::SEMICOL, "; tras campo");
    }
    consume(Token::RBRACE, "'}' struct");
}

void Parser::parseTypeAlias(){
    consume(Token::TYPE, "'type'");
    consume(Token::IDENTIFIER, "nombre alias");
    consume(Token::ASSIGN, "'=' en alias de tipo");
    // Type
    if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::BOOL)) {
        if (match(Token::LBRACKET)){
            consume(Token::NUMBER, "tamaño de array");
            consume(Token::RBRACKET, "]");
        }
    } else throw runtime_error("Tipo esperado en alias");
    consume(Token::SEMICOL, "; final alias");
}

BlockStm* Parser::parseBlock(){
    BlockStm* block = new BlockStm();
    consume(Token::LBRACE, "'{' bloque");
    while(!check(Token::RBRACE) && !isAtEnd()){
        Stm* s = parseStatement();
        if (s) block->statements.push_back(s);
    }
    consume(Token::RBRACE, "'}' bloque");
    return block;
}

Stm* Parser::parseStatement(){
    if (check(Token::LET))   return parseVarDecl();
    if (check(Token::IF))    return parseIf();
    if (check(Token::WHILE)) return parseWhile();
    if (check(Token::FOR))   return parseFor();
    if (check(Token::RETURN))return parseReturn();
    if (check(Token::PRINTLN)) return parsePrint();
    if (check(Token::LBRACE)) return parseBlock();
    // ExpressionStmt
    Exp* e = parseExpression();
    if (match(Token::SEMICOL)) return new AssignStm("_", e); // placeholder simple
    // Permitir expresión final de bloque como retorno implícito
    if (check(Token::RBRACE)) {
        ReturnStm* r = new ReturnStm();
        r->e = e;
        return r;
    }
    throw runtime_error("Error sintáctico: se esperaba ';' o fin de bloque tras expresión");
}

LetStm* Parser::parseVarDecl(){
    consume(Token::LET, "'let'");
    bool mut = match(Token::MUT);
    consume(Token::IDENTIFIER, "nombre variable");
    string varName = previous->text;
    consume(Token::COLON, ": en declaración");
    // Type (guardamos solo como texto simple por ahora)
    string typeName;
    if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::BOOL)) {
        typeName = previous->text;
        if (match(Token::LBRACKET)) { consume(Token::NUMBER, "tamaño array"); consume(Token::RBRACKET, "]"); }
    } else throw runtime_error("Tipo esperado en declaración");
    // OptAssign
    Exp* init = nullptr;
    if (match(Token::ASSIGN)) {
        init = parseExpression();
    }
    consume(Token::SEMICOL, "; final declaración");
    return new LetStm(mut, varName, typeName, init);
}

IfStm* Parser::parseIf(){
    consume(Token::IF, "'if'");
    Exp* cond = nullptr;
    if (match(Token::LPAREN)){
        cond = parseExpression(); consume(Token::RPAREN, ") en if");
    } else {
        cond = parseExpression(); // forma sin paréntesis
    }
    BlockStm* thenB = parseBlock();
    BlockStm* elseB = nullptr;
    if (match(Token::ELSE)) { elseB = parseBlock(); }
    return new IfStm(cond, thenB, elseB);
}

WhileStm* Parser::parseWhile(){
    consume(Token::WHILE, "'while'");
    Exp* cond = nullptr;
    if (match(Token::LPAREN)){
        cond = parseExpression(); consume(Token::RPAREN, ") en while");
    } else { cond = parseExpression(); }
    BlockStm* body = parseBlock();
    return new WhileStm(cond, body);
}

ForStm* Parser::parseFor(){
    consume(Token::FOR, "'for'");
    consume(Token::IDENTIFIER, "iterador for");
    string it = previous->text;
    consume(Token::IN, "'in' en for");
    Exp* start = parseExpression();
    consume(Token::DOTDOT, "'..' rango for");
    Exp* end = parseExpression();
    BlockStm* body = parseBlock();
    return new ForStm(it, start, end, body);
}

ReturnStm* Parser::parseReturn(){
    consume(Token::RETURN, "'return'");
    ReturnStm* r = new ReturnStm();
    // ExpressionOpt
    if (!check(Token::SEMICOL)) { r->e = parseExpression(); }
    consume(Token::SEMICOL, "; en return");
    return r;
}

PrintStm* Parser::parsePrint(){
    consume(Token::PRINTLN, "'println!'");
    consume(Token::LPAREN, "'(' en println");
    // STRING_LITERAL opcional seguido de , lista de expresiones
    if (check(Token::STRING_LITERAL)) advance();
    Exp* firstExpr = nullptr;
    if (match(Token::COMMA)) {
        // lista de expresiones
        firstExpr = parseExpression();
        // si hay múltiples expresiones, por ahora ignoramos las adicionales
        while(match(Token::COMMA)) { Exp* e2 = parseExpression(); (void)e2; }
    }
    consume(Token::RPAREN, ") en println");
    consume(Token::SEMICOL, "; en println");
    return new PrintStm(firstExpr);
}

// =====================
// Expresiones
// =====================
Exp* Parser::parseExpression(){ return parseAssignment(); }

Exp* Parser::parseAssignment(){
    Exp* left = parseOr();
    if (match(Token::ASSIGN) || match(Token::PLUS_ASSIGN) || match(Token::MINUS_ASSIGN)){
        Exp* right = parseAssignment(); (void)right; // ignoramos construcción
    }
    return left;
}

Exp* Parser::parseOr(){
    Exp* left = parseAnd();
    while (match(Token::OR)) { Exp* right = parseAnd(); left = new BinaryExp(left, right, AND_OP); }
    return left;
}

Exp* Parser::parseAnd(){
    Exp* left = parseRel();
    while (match(Token::AND)) { Exp* right = parseRel(); left = new BinaryExp(left, right, AND_OP); }
    return left;
}

Exp* Parser::parseRel(){
    Exp* left = parseAdd();
    while (match(Token::EQ) || match(Token::NEQ) || match(Token::LT) || match(Token::GT) || match(Token::LE) || match(Token::GE)) {
        Exp* right = parseAdd(); left = new BinaryExp(left, right, LE_OP); }
    return left;
}

Exp* Parser::parseAdd(){
    Exp* left = parseMul();
    while (match(Token::PLUS) || match(Token::MINUS)) { Exp* right = parseMul(); left = new BinaryExp(left, right, PLUS_OP); }
    return left;
}

Exp* Parser::parseMul(){
    Exp* left = parseUnary();
    while (match(Token::MUL) || match(Token::DIV) || match(Token::MOD)) { Exp* right = parseUnary(); left = new BinaryExp(left, right, MUL_OP); }
    return left;
}

Exp* Parser::parseUnary(){
    if (match(Token::NOT) || match(Token::MINUS) || match(Token::PLUS)) { Exp* right = parseUnary(); return right; }
    return parsePostfix();
}

Exp* Parser::parsePostfix(){
    Exp* primary = parsePrimary();
    while (true){
        if (match(Token::DOT)) { consume(Token::IDENTIFIER, "identificador tras '.'"); continue; }
        if (match(Token::LBRACKET)) { Exp* idx = parseExpression(); (void)idx; consume(Token::RBRACKET, "] en indexación"); continue; }
        if (match(Token::LPAREN)) {
            // ArgListOpt
            if (!check(Token::RPAREN)) {
                Exp* a = parseExpression(); (void)a;
                while(match(Token::COMMA)) { Exp* b = parseExpression(); (void)b; }
            }
            consume(Token::RPAREN, ") cierre llamada");
            continue;
        }
        break;
    }
    return primary;
}

Exp* Parser::parsePrimary(){
    if (match(Token::NUMBER)) return new NumberExp(stoi(previous->text));
    if (match(Token::TRUE)) { BoolExp* b = new BoolExp(); b->valor = 1; return b; }
    if (match(Token::FALSE)) { BoolExp* b = new BoolExp(); b->valor = 0; return b; }
    if (match(Token::IDENTIFIER)) {
        string name = previous->text;
        return new IdExp(name);
    }
    if (match(Token::LPAREN)) { Exp* e = parseExpression(); consume(Token::RPAREN, ") cierre"); return e; }
    throw runtime_error("Expresión primaria inesperada");
}
