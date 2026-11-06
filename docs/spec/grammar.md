# Appendix A. Grammar

This grammar describes the language as specified in version 0.13. `{ x }` means zero or more repetitions, `[ x ]` means optional, and quoted text is a literal token. `//` and `/* */` comments and whitespace may appear between tokens.

```text
file           = { import } { topDecl } ;
import         = "import" IMPORT_PATH ";" ;
topDecl        = namespaceDecl | decl ;
namespaceDecl  = "namespace" IDENT "{" { decl } "}" ;
decl           = functionDecl | structDecl | interfaceDecl | enumDecl
               | constDecl | globalDecl | externBlock ;
globalDecl     = typeName IDENT "=" expr ";" ;                         (* global object *)

functionDecl   = returnType IDENT "(" [ params ] ")" block ;
params         = param { "," param } ;
param          = type [ "*" ] IDENT ;                    (* "*": reference *)
returnType     = "void" | type [ "*" ] ;                 (* "*": returns a reference *)

structDecl     = "struct" IDENT [ ":" typeName { "," typeName } ] "{" { member } "}" ;
member         = [ "private" ] type IDENT ";"                          (* field *)
               | [ "private" ] returnType IDENT "(" [ params ] ")" block   (* method *)
               | IDENT "(" [ params ] ")" block ;                      (* constructor *)
interfaceDecl  = "interface" IDENT "{" { returnType IDENT "(" [ params ] ")" ";" } "}" ;
enumDecl       = "enum" IDENT "{" IDENT { "," IDENT } [ "," ] "}" ;
constDecl      = "const" type IDENT "=" constValue ";" ;
constValue     = [ "-" ] literal | typeName "." IDENT ;
externBlock    = "extern" STRING "{" { externFunction } "}" ;
externFunction = ( "void" | type ) IDENT "(" [ externParams ] ")" ";" ;
externParams   = "..." | externParam { "," externParam } [ "," "..." ] ;
externParam    = type [ "*" ] IDENT ;                      (* "*" only for String *)

type           = baseType { "[" "]" } ;
baseType       = primitiveType | "String" | typeName ;
primitiveType  = "boolean" | "char" | "int8" | "int16" | "int" | "int64"
               | "uint8" | "uint16" | "uint" | "uint64" | "float" | "float64" ;
typeName       = IDENT [ "." IDENT ] ;

block          = "{" { statement } "}" ;
statement      = block | varDecl | ifStmt | whileStmt | doStmt | forStmt
               | switchStmt | "break" ";" | "continue" ";" | "return" [ expr ] ";"
               | simpleStmt ";" ;
varDecl        = type IDENT [ "=" initialiser ] ";"
               | type "*" IDENT "=" expr ";" ;                         (* reference variable *)
initialiser    = expr | "{" initialiser { "," initialiser } [ "," ] "}" ;
simpleStmt     = expr [ assignOp expr | "++" | "--" ] | ( "++" | "--" ) expr ;
assignOp       = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ;
ifStmt         = "if" "(" expr ")" statement [ "else" statement ] ;
whileStmt      = "while" "(" expr ")" statement ;
doStmt         = "do" statement "while" "(" expr ")" ";" ;
forStmt        = "for" "(" [ forInit ] ";" [ expr ] ";" [ simpleStmt ] ")" statement ;
forInit        = type [ "*" ] IDENT "=" expr | simpleStmt ;
switchStmt     = "switch" "(" expr ")" "{" { switchLabel "->" statement } "}" ;
switchLabel    = "case" caseValue { "," caseValue } | "default" ;
caseValue      = IDENT | [ "-" ] INTEGER | CHARACTER | STRING | "true" | "false" ;

expr           = orExpr ;
orExpr         = andExpr { "||" andExpr } ;
andExpr        = bitOrExpr { "&&" bitOrExpr } ;
bitOrExpr      = bitXorExpr { "|" bitXorExpr } ;
bitXorExpr     = bitAndExpr { "^" bitAndExpr } ;
bitAndExpr     = equalityExpr { "&" equalityExpr } ;
equalityExpr   = relationalExpr { ( "==" | "!=" ) relationalExpr } ;
relationalExpr = shiftExpr { ( "<" | "<=" | ">" | ">=" ) shiftExpr } ;
shiftExpr      = additiveExpr { ( "<<" | ">>" ) additiveExpr } ;
additiveExpr   = multiplicativeExpr { ( "+" | "-" ) multiplicativeExpr } ;
multiplicativeExpr = unaryExpr { ( "*" | "/" | "%" ) unaryExpr } ;
unaryExpr      = ( "-" | "!" | "~" ) unaryExpr
               | "(" primitiveType ")" unaryExpr                       (* cast *)
               | postfixExpr ;
postfixExpr    = primary { "." IDENT | "[" expr "]" | "(" [ args ] ")" } ;
args           = expr { "," expr } ;
primary        = literal | IDENT | "this" | "(" expr ")" | switchExpr
               | "String" "." IDENT                                     (* String.of *)
               | baseType "[" expr "]" { "[" expr "]" } ;              (* new array *)
switchExpr     = "switch" "(" expr ")" "{" { switchLabel "->" expr ";" } "}" ;
literal        = INTEGER | FLOAT | CHARACTER | STRING | "true" | "false" ;
```

`IMPORT_PATH` is the text between `import` and `;`, with surrounding whitespace removed. A statement that starts with `switch` is a switch statement. A statement that starts with a type followed by an identifier, or by `*`, an identifier and `=`, is a variable declaration. In an expression, `Name[n]` creates an array when `Name` is a type; when it is a variable, it is an element.

---

[← 20. Project identity](20-identity.md) · [Contents](README.md)
