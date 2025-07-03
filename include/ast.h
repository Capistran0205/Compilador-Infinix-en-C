// ast.h - Definición del Árbol Sintáctico

#ifndef AST_H
#define AST_H

#include "tipos.h"

// Tipos de nodos del árbol sintáctico
typedef enum {
    // Declaraciones
    NODE_DECLARATION,
    NODE_ASSIGNMENT,
    
    // Expresiones
    NODE_EXPRESSION,
    NODE_BINARY_OP,     // +, -, *, /, <, >
    NODE_UNARY_OP,      // - (negativo)
    
    // Literales
    NODE_NUMBER,
    NODE_STRING,
    NODE_CHAR,
    NODE_BOOLEAN,
    NODE_IDENTIFIER,
    
    // Sentencias
    NODE_INPUT,         // inp
    NODE_OUTPUT,        // mt
    
    // Estructura
    NODE_PROGRAM,       // Raíz del programa
    NODE_STATEMENT_LIST // Lista de sentencias
} NodeType;

// Estructura del nodo del árbol
typedef struct ASTNode {
    NodeType type;
    char value[MAX_LINE_LENGTH];        // Valor del nodo (lexema)
    TokenType token_type;               // Tipo de token original
    int line_number;                    // Línea donde aparece
    
    // Hijos del nodo
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *third;              // Para nodos ternarios (ej: declaraciones)
    
    // Para listas de sentencias
    struct ASTNode **children;          // Array de hijos
    int child_count;
    int child_capacity;
} ASTNode;

// Funciones del AST
ASTNode* create_node(NodeType type, const char* value, int line);
ASTNode* create_binary_node(NodeType type, ASTNode* left, ASTNode* right);
ASTNode* create_declaration_node(const char* type, const char* name, ASTNode* value);
void add_child(ASTNode* parent, ASTNode* child);
void print_ast(ASTNode* node, int depth);
void free_ast(ASTNode* node);

#endif // AST_H