# Gramática Rust Simplificada

Subconjunto de la gramática de Rust extraída del codigo del parser de IntelliJ que tiene las siguientes características:

* Declaración de variables
* Operaciones aritméticas y relacionales
* Sentencias de control (`if`, `while`)
* Definición y uso de funciones
* Tipos primitivos y estructuras (`u32`, `f32`, `struct`)
* Alias de tipo (`type` → similar a `typedef`)
* Retorno de estructuras o arreglos

**Esta gramática es LR(1), fue probado con Bison**

```
Grammar

    0 $accept: Program $end

    1 Program: Items

    2 Items: ε
    3      | Items Item

    4 Item: FunctionDecl
    5     | StructDecl
    6     | TypeAliasDecl

    7 FunctionDecl: FN IDENTIFIER '(' ParamListOpt ')' Block

    8 ParamListOpt: ε
    9             | ParamList

   10 ParamList: Param
   11          | ParamList ',' Param

   12 Param: IDENTIFIER ':' Type

   13 StructDecl: STRUCT IDENTIFIER '{' StructFieldListOpt '}'

   14 StructFieldListOpt: ε
   15                   | StructFieldList

   16 StructFieldList: StructField
   17                | StructFieldList StructField

   18 StructField: IDENTIFIER ':' Type ';'

   19 TypeAliasDecl: TYPE IDENTIFIER '=' Type ';'

   20 Statement: VarDecl
   21          | ExpressionStmt
   22          | IfStmt
   23          | ForStmt
   24          | WhileStmt
   25          | ReturnStmt
   26          | PrintStmt
   27          | Block

   28 VarDecl: LET MutOpt IDENTIFIER ':' Type OptAssign ';'

   29 MutOpt: ε
   30       | MUT

   31 OptAssign: ε
   32          | '=' Expression

   33 ExpressionStmt: Expression ';'

   34 ForStmt: FOR IDENTIFIER IN Expression DOTDOT Expression Block

   35 IfStmt: IF '(' Expression ')' Block ElseOpt

   36 ElseOpt: ε
   37        | ELSE Block

   38 WhileStmt: WHILE '(' Expression ')' Block

   39 ReturnStmt: RETURN ExpressionOpt ';'

   40 ExpressionOpt: ε
   41              | Expression

   42 PrintStmt: PRINTLN! '(' STRING_LITERAL PrintArgsOpt ')' ';'

   43 PrintArgsOpt: ε
   44             | ',' ExpressionList

   45 Block: '{' StatementListOpt '}'

   46 StatementListOpt: ε
   47                 | StatementList

   48 StatementList: Statement
   49              | StatementList Statement

   50 Type: PrimitiveType
   51     | IDENTIFIER
   52     | ArrayType

   53 PrimitiveType: U8
   54              | U16
   55              | U32
   56              | U64
   58              | I32
   59              | I64
   60              | F32
   61              | F64
   62              | BOOL

   63 ArrayType: Type '[' NUMBER ']'

   64 Expression: Assignment

   65 Assignment: OrExpr
   66           | OrExpr '=' Assignment
   67           | OrExpr PLUS_ASSIGN Assignment
   68           | OrExpr MINUS_ASSIGN Assignment

   69 OrExpr: AndExpr
   70       | OrExpr OR AndExpr

   71 AndExpr: RelExpr
   72        | AndExpr AND RelExpr

   73 RelExpr: AddExpr
   74        | RelExpr EQ AddExpr
   75        | RelExpr NEQ AddExpr
   76        | RelExpr '<' AddExpr
   77        | RelExpr '>' AddExpr
   78        | RelExpr LE AddExpr
   79        | RelExpr GE AddExpr

   80 AddExpr: MulExpr
   81        | AddExpr '+' MulExpr
   82        | AddExpr '-' MulExpr

   83 MulExpr: UnaryExpr
   84        | MulExpr '*' UnaryExpr
   85        | MulExpr '/' UnaryExpr
   86        | MulExpr '%' UnaryExpr

   87 UnaryExpr: PostfixExpr
   88          | '-' UnaryExpr
   89          | '+' UnaryExpr
   90          | '!' UnaryExpr

   91 PostfixExpr: Primary
   92            | PostfixExpr '.' IDENTIFIER
   93            | PostfixExpr '[' Expression ']'
   94            | PostfixExpr '(' ArgListOpt ')'

   95 Primary: NUMBER
   96        | IDENTIFIER
   97        | StructInit
   98        | '(' Expression ')'

   99 StructInit: IDENTIFIER '{' StructInitFieldListOpt '}'

  100 StructInitFieldListOpt: ε
  101                       | StructInitFieldList

  102 StructInitFieldList: StructInitField
  103                    | StructInitFieldList ',' StructInitField

  104 StructInitField: IDENTIFIER ':' Expression

  105 ArgListOpt: ε
  106           | ArgList

  107 ArgList: Expression
  108        | ArgList ',' Expression

  109 ExpressionList: Expression
  110               | ExpressionList ',' Expression
```