// ast.c - Implementación del Árbol Sintáctico

#include "ast.h"
#include <stdlib.h>
#include <string.h>

/**
 * Crear un nuevo nodo del AST
 */
ASTNode* create_node(NodeType type, const char* value, int line) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    strcpy(node->value, value ? value : "");
    node->line_number = line;
    node->left = NULL;
    node->right = NULL;
    node->third = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    
    return node;
}

/**
 * Crear un nodo binario (operadores)
 */
ASTNode* create_binary_node(NodeType type, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_node(type, "", 0);
    node->left = left;
    node->right = right;
    return node;
}

/**
 * Crear un nodo de declaración
 */
ASTNode* create_declaration_node(const char* type, const char* name, ASTNode* value) {
    ASTNode* node = create_node(NODE_DECLARATION, name, 0);
    
    // Crear nodo para el tipo
    node->left = create_node(NODE_IDENTIFIER, type, 0);
    
    // Asignar valor inicial si existe
    if (value) {
        node->right = value;
    }
    
    return node;
}

/**
 * Agregar un hijo a un nodo
 */
void add_child(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;
    
    // Inicializar array de hijos si es necesario
    if (parent->children == NULL) {
        parent->child_capacity = 4;
        parent->children = malloc(sizeof(ASTNode*) * parent->child_capacity);
    }
    
    // Expandir array si es necesario
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = realloc(parent->children, 
                                   sizeof(ASTNode*) * parent->child_capacity);
    }
    
    parent->children[parent->child_count++] = child;
}

/**
 * Obtener nombre del tipo de nodo
 */
const char* get_node_type_name(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_DECLARATION: return "DECLARATION";
        case NODE_ASSIGNMENT: return "ASSIGNMENT";
        case NODE_EXPRESSION: return "EXPRESSION";
        case NODE_BINARY_OP: return "BINARY_OP";
        case NODE_UNARY_OP: return "UNARY_OP";
        case NODE_NUMBER: return "NUMBER";
        case NODE_STRING: return "STRING";
        case NODE_CHAR: return "CHAR";
        case NODE_BOOLEAN: return "BOOLEAN";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_INPUT: return "INPUT";
        case NODE_OUTPUT: return "OUTPUT";
        case NODE_STATEMENT_LIST: return "STATEMENT_LIST";
        default: return "UNKNOWN";
    }
}

/**
 * Imprimir el AST con indentación
 */
void print_ast(ASTNode* node, int depth) {
    if (!node) return;
    
    // Imprimir indentación
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Imprimir información del nodo
    printf("%s", get_node_type_name(node->type));
    if (strlen(node->value) > 0) {
        printf(": %s", node->value);
    }
    if (node->line_number > 0) {
        printf(" (línea %d)", node->line_number);
    }
    printf("\n");
    
    // Imprimir hijos binarios
    if (node->left) {
        print_ast(node->left, depth + 1);
    }
    if (node->right) {
        print_ast(node->right, depth + 1);
    }
    if (node->third) {
        print_ast(node->third, depth + 1);
    }
    
    // Imprimir lista de hijos
    for (int i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], depth + 1);
    }
}

/**
 * Liberar memoria del AST
 */
void free_ast(ASTNode* node) {
    if (!node) return;
    
    // Liberar hijos binarios
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->third);
    
    // Liberar lista de hijos
    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    if (node->children) {
        free(node->children);
    }
    
    free(node);
}