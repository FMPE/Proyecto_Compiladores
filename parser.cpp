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
            p->sdlist.push_back(parseStruct());
        } else if (check(Token::TYPE)) {
            p->talist.push_back(parseTypeAlias());
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
        if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::U64) || match(Token::F32) || match(Token::F64) || match(Token::BOOL)) {
            paramN.push_back(pname);
            paramT.push_back(previous->text);
            while(match(Token::COMMA)){
                consume(Token::IDENTIFIER, "param nombre");
                pname = previous->text;
                consume(Token::COLON, ":");
                if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::U64) || match(Token::F32) || match(Token::F64) || match(Token::BOOL)) {
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
        if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::U64) || match(Token::F32) || match(Token::F64) || match(Token::BOOL)){
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

StructDec* Parser::parseStruct(){
    consume(Token::STRUCT, "'struct'");
    consume(Token::IDENTIFIER, "nombre de struct");
    string structName = previous->text;
    StructDec* sd = new StructDec(structName);
    consume(Token::LBRACE, "'{' struct");
    // Campos: IDENT ':' Type ';'
    while(check(Token::IDENTIFIER)){
        advance(); // nombre campo
        string fieldName = previous->text;
        consume(Token::COLON, ": en campo struct");
        // Type simplificada
        string fieldType;
        if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::U64) || match(Token::F32) || match(Token::F64) || match(Token::BOOL)) {
            fieldType = previous->text;
            // ArrayType opcional
            if (match(Token::LBRACKET)){
                string size = current->text;
                consume(Token::NUMBER, "tamaño array en campo struct");
                consume(Token::RBRACKET, "]");
                fieldType += "[" + size + "]";
            }
        } else throw runtime_error("Tipo esperado en campo struct");
        consume(Token::SEMICOL, "; tras campo");
        sd->fields.push_back({fieldName, fieldType});
    }
    consume(Token::RBRACE, "'}' struct");
    return sd;
}

TypeAlias* Parser::parseTypeAlias(){
    consume(Token::TYPE, "'type'");
    consume(Token::IDENTIFIER, "nombre alias");
    string alias = previous->text;
    consume(Token::ASSIGN, "'=' en alias de tipo");
    // Type
    string typeName;
    if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::U64) || match(Token::F32) || match(Token::F64) || match(Token::BOOL)) {
        typeName = previous->text;
        if (match(Token::LBRACKET)){
            consume(Token::NUMBER, "tamaño de array");
            typeName += "[" + previous->text + "]";
            consume(Token::RBRACKET, "]");
        }
    } else throw runtime_error("Tipo esperado en alias");
    consume(Token::SEMICOL, "; final alias");
    return new TypeAlias(alias, typeName);
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
    if (match(Token::IDENTIFIER) || match(Token::I32) || match(Token::I64) || match(Token::U32) || match(Token::U64) || match(Token::F32) || match(Token::F64) || match(Token::BOOL)) {
        typeName = previous->text;
        if (match(Token::LBRACKET)) { 
            string size = current->text;
            consume(Token::NUMBER, "tamaño array"); 
            consume(Token::RBRACKET, "]"); 
            typeName += "[" + size + "]";
        }
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
    if (match(Token::ASSIGN)){
        Exp* right = parseAssignment();
        return new BinaryExp(left, right, ASSIGN_OP);
    }
    if (match(Token::PLUS_ASSIGN)){
        Exp* right = parseAssignment();
        IdExp* idLeft = dynamic_cast<IdExp*>(left);
        if (idLeft) {
            Exp* copyLeft = new IdExp(idLeft->value);
            Exp* addExp = new BinaryExp(copyLeft, right, PLUS_OP);
            return new BinaryExp(left, addExp, ASSIGN_OP);
        }
        throw runtime_error("Compound assignment += requires identifier on left side");
    }
    if (match(Token::MINUS_ASSIGN)){
        Exp* right = parseAssignment();
        IdExp* idLeft = dynamic_cast<IdExp*>(left);
        if (idLeft) {
            Exp* copyLeft = new IdExp(idLeft->value);
            Exp* subExp = new BinaryExp(copyLeft, right, MINUS_OP);
            return new BinaryExp(left, subExp, ASSIGN_OP);
        }
        throw runtime_error("Compound assignment -= requires identifier on left side");
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
        BinaryOp op;
        switch(previous->type){
            case Token::EQ: op = EQ_OP; break;
            case Token::NEQ: op = NEQ_OP; break;
            case Token::LT: op = LT_OP; break;
            case Token::GT: op = GT_OP; break;
            case Token::LE: op = LE_OP; break;
            case Token::GE: op = GE_OP; break;
            default: op = LE_OP; break;
        }
        Exp* right = parseAdd(); 
        left = new BinaryExp(left, right, op); 
    }
    return left;
}

Exp* Parser::parseAdd(){
    Exp* left = parseMul();
    while (match(Token::PLUS) || match(Token::MINUS)) { 
        BinaryOp op = (previous->type == Token::PLUS) ? PLUS_OP : MINUS_OP;
        Exp* right = parseMul(); 
        left = new BinaryExp(left, right, op); 
    }
    return left;
}

Exp* Parser::parseMul(){
    Exp* left = parseUnary();
    while (match(Token::MUL) || match(Token::DIV)) { 
        BinaryOp op = (previous->type == Token::MUL) ? MUL_OP : DIV_OP;
        Exp* right = parseUnary(); 
        left = new BinaryExp(left, right, op); 
    }
    return left;
}

Exp* Parser::parseUnary(){
    if (match(Token::NOT) || match(Token::MINUS) || match(Token::PLUS)) { Exp* right = parseUnary(); return right; }
    return parsePostfix();
}

Exp* Parser::parsePostfix(){
    Exp* primary = parsePrimary();
    while (true){
        if (match(Token::DOT)) { 
            consume(Token::IDENTIFIER, "identificador tras '.'"); 
            primary = new FieldAccessExp(primary, previous->text);
            continue; 
        }
        if (match(Token::LBRACKET)) { 
            Exp* index = parseExpression(); 
            consume(Token::RBRACKET, "] en indexación"); 
            primary = new ArrayAccessExp(primary, index);
            continue; 
        }
        if (match(Token::LPAREN)) {
            FcallExp* fcall = new FcallExp();
            if (IdExp* id = dynamic_cast<IdExp*>(primary)) {
                fcall->nombre = id->value;
                // Don't delete primary here if we casted it? 
                // Actually we are replacing 'primary' pointer with 'fcall'.
                // The 'id' pointer points to the same memory.
                // We should NOT delete 'id' yet if we want to use its value, but we copied the value.
                // But 'primary' was allocated with 'new IdExp'. We should delete it to avoid leak, 
                // BUT 'fcall' is the new primary.
                // Wait, 'primary' is the 'IdExp'. We extract the name. Then we can delete 'primary'.
                // But 'id' is just a casted pointer to 'primary'.
                // So:
                // string name = id->value;
                // delete primary; 
                // fcall->nombre = name;
                // But wait, IdExp destructor might be virtual? Exp has virtual destructor.
                // Yes.
            } else {
                throw runtime_error("Llamada a función requiere identificador");
            }
            
            // We need to be careful. If we delete primary, 'id' becomes invalid?
            // Yes. So extract name first.
            string funcName = ((IdExp*)primary)->value;
            delete primary;
            fcall->nombre = funcName;

            if (!check(Token::RPAREN)) {
                fcall->argumentos.push_back(parseExpression());
                while(match(Token::COMMA)) { 
                    fcall->argumentos.push_back(parseExpression()); 
                }
            }
            consume(Token::RPAREN, ") cierre llamada");
            primary = fcall;
            continue;
        }
        if (check(Token::LBRACE)) {
            // Struct initialization: Point { x: 1, y: 2 }
            // primary must be IdExp
            if (IdExp* id = dynamic_cast<IdExp*>(primary)) {
                advance(); // Consume LBRACE
                StructInitExp* sinit = new StructInitExp("");
                sinit->name = id->value;
                delete primary;
                
                // Parse fields: ident : expr , ...
                if (!check(Token::RBRACE)) {
                    do {
                        consume(Token::IDENTIFIER, "nombre campo struct");
                        string fieldName = previous->text;
                        consume(Token::COLON, ": en campo struct");
                        Exp* val = parseExpression();
                        sinit->fields.push_back({fieldName, val});
                    } while(match(Token::COMMA));
                }
                consume(Token::RBRACE, "} cierre struct init");
                primary = sinit;
                continue;
            }
            // If not IdExp, do not consume LBRACE (it's likely start of a block)
        }
        break;
    }
    return primary;
}

Exp* Parser::parsePrimary(){
    if (match(Token::NUMBER)) {
        string text = previous->text;
        if (text.find('.') != string::npos) {
            return new FloatExp(stod(text), true); // Default to double/f64 for literals
        }
        return new NumberExp(stoll(text));
    }
    if (match(Token::TRUE)) { BoolExp* b = new BoolExp(); b->valor = 1; return b; }
    if (match(Token::FALSE)) { BoolExp* b = new BoolExp(); b->valor = 0; return b; }
    if (match(Token::IDENTIFIER)) {
        string name = previous->text;
        return new IdExp(name);
    }
    if (match(Token::LPAREN)) { Exp* e = parseExpression(); consume(Token::RPAREN, ") cierre"); return e; }
    throw runtime_error("Expresión primaria inesperada");
}
