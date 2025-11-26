#ifndef PARSER_H       
#define PARSER_H

#include "scanner.h"
#include "ast.h"

class Parser {
private:
    Scanner* scanner;
    Token* current;
    Token* previous;

    // utilidades
    bool match(Token::Type t);
    bool check(Token::Type t);
    void advance();
    bool isAtEnd();
    void consume(Token::Type t, const string& msg);

    // producciones principales
    void parseItems(Program* p);
    void parseItem(Program* p);
    FunDec* parseFunction();
    StructDec* parseStruct(); 
    TypeAlias* parseTypeAlias(); // placeholder

    // statements / bloques
    BlockStm* parseBlock();
    Stm* parseStatement();
    LetStm* parseVarDecl();
    IfStm* parseIf();
    WhileStm* parseWhile();
    ForStm* parseFor();
    ReturnStm* parseReturn();
    PrintStm* parsePrint();

    // expresiones
    Exp* parseExpression();
    Exp* parseAssignment();
    Exp* parseOr();
    Exp* parseAnd();
    Exp* parseRel();
    Exp* parseAdd();
    Exp* parseMul();
    Exp* parseUnary();
    Exp* parsePostfix();
    Exp* parsePrimary();

public:
    Parser(Scanner* sc);
    Program* parseProgram();
};

#endif // PARSER_H      