#include "parser.h"

int current_token_index = 0;
Token current_token_tree;

/**
 * Obtener el token actual sin avanzar
 */
Token peek_token() {
    if (current_token_index < token_count) {
        return tokens[current_token_index];
    }
    // Token EOF
    Token eof_token = {TOKEN_EOF, "EOF", -1};
    return eof_token;
}

/**
 * Consumir el token actual y avanzar al siguiente
 */
Token consume_token() {
    if (current_token_index < token_count) {
        return tokens[current_token_index++];
    }
    Token eof_token = {TOKEN_EOF, "EOF", -1};
    return eof_token;
}

/**
 * Verificar si el token actual coincide con el esperado
 */
bool match_token(TokenType expected) {
    return peek_token().type == expected;
}

/**
 * Verificar si es un token de tipo de dato
 */
bool is_type_token(TokenType type) {
    return (type == TOKEN_ENTERO || type == TOKEN_DECIMAL || 
            type == TOKEN_CARACTER || type == TOKEN_CADENA || 
            type == TOKEN_BOOLEANO);
}

/**
 * Parsear el programa completo
 * Programa -> Sentencia*
 */
ASTNode* parse_program_tree() {
    ASTNode* program = create_node(NODE_PROGRAM, "PROGRAM", 1);
    
    while (peek_token().type != TOKEN_EOF) {
        ASTNode* statement = parse_statement_tree();
        if (statement) {
            add_child(program, statement);
        }
    }
    
    return program;
}

/**
 * Parsear una sentencia
 * Sentencia -> Declaracion | Asignacion | Entrada | Salida
 */
ASTNode* parse_statement_tree() {
    Token token = peek_token();
    
    // Declaración: tipo identificador [= expresion] :_:
    if (is_type_token(token.type)) {
        return parse_declaration_tree();
    }
    
    // Entrada: inp < expresion > :_:
    if (token.type == TOKEN_ENTRADA) {
        return parse_input_tree();
    }
    
    // Salida: mt < expresion > :_:
    if (token.type == TOKEN_SALIDA) {
        return parse_output_tree();
    }
    
    // Asignación: identificador = expresion :_:
    if (token.type == TOKEN_IDENTIFICADOR) {
        return parse_assignment_tree();
    }
    
    // Error: token no reconocido
    /*printf("Error sintáctico en línea %d: token inesperado '%s'\n", 
           token.linea, token.lexema);*/
    consume_token(); // Saltar token problemático
    return NULL;
}

/**
 * Parsear declaración
 * Declaracion -> Tipo Identificador [= Expresion] :_:
 */
ASTNode* parse_declaration_tree() {
    Token type_token = consume_token(); // entero, decimal, etc.
    
    if (!match_token(TOKEN_IDENTIFICADOR)) {        
        return NULL;
    }
    
    Token id_token = consume_token();
    ASTNode* value_node = NULL;
    
    // Inicialización opcional
    if (match_token(TOKEN_IGUAL)) {
        consume_token(); // Consumir '='
        value_node = parse_expression_tree();
    }
    
    // Verificar fin de instrucción
    if (!match_token(TOKEN_FINLINEA)) {        
        return NULL;
    }
    consume_token(); // Consumir ':_:'    
    
    return create_declaration_node(type_token.lexema, id_token.lexema, value_node);
}

/**
 * Parsear expresión simple
 * Expresion -> Termino (Operador Termino)*
 */
ASTNode* parse_expression_tree() {
    ASTNode* left = parse_term_tree();
    
    while (match_token(TOKEN_MAS) || match_token(TOKEN_MENOS) ||
           match_token(TOKEN_MENOR) || match_token(TOKEN_MAYOR)) {
        Token op = consume_token();
        ASTNode* right = parse_term_tree();
        left = create_binary_node(NODE_BINARY_OP, left, right);
        strcpy(left->value, op.lexema);
    }
    
    return left;
}

ASTNode* parse_assignment_tree(){        
    // Consume el token Identificador
    Token id_token = consume_token();
    // Raíz
    ASTNode* value_node = NULL;

    if(match_token(TOKEN_IGUAL)){
        if (match_token(TOKEN_IGUAL)) {
        consume_token(); // Consumir '='
        value_node = parse_expression_tree();    
        }
    }
    // Verificar fin de instrucción
    if (!match_token(TOKEN_FINLINEA)) {        
        return NULL;
    }
    consume_token(); // Consumir ':_:'

    // Crear nodo de asignación
    ASTNode* assignment_node = create_node(NODE_ASSIGNMENT, id_token.lexema, id_token.linea);
    add_child(assignment_node, value_node);
    
    return assignment_node;
}

ASTNode* parse_input_tree() {
    // Consumir el token 'inp'
    Token inp_token = consume_token();
    
    // Verificar token '<'
    if (!match_token(TOKEN_MENOR)) {       
        return NULL;
    }
    consume_token(); // Consumir '<'
    
    // Parsear la expresión (normalmente un identificador para entrada)
    ASTNode* target_node = parse_expression_tree();
    if (!target_node) {
        return NULL;
    }
    
    // Verificar token '>'
    if (!match_token(TOKEN_MAYOR)) {        
        return NULL;
    }
    consume_token(); // Consumir '>'
    
    // Verificar fin de instrucción
    if (!match_token(TOKEN_FINLINEA)) {        
        return NULL;
    }
    consume_token(); // Consumir ':_:'
    
    // Crear nodo de entrada
    ASTNode* input_node = create_node(NODE_INPUT, inp_token.lexema, inp_token.linea);
    add_child(input_node, target_node);
    
    return input_node;
}

/**
 * Parsear salida
 * Salida -> mt < Expresion > :_:
 */
ASTNode* parse_output_tree() {
    // Consumir el token 'mt'
    Token mt_token = consume_token();
    
    // Verificar token '<'
    if (!match_token(TOKEN_MENOR)) {        
        return NULL;
    }
    consume_token(); // Consumir '<'
    
    // Parsear la expresión a mostrar
    ASTNode* expr_node = parse_expression_tree();
    if (!expr_node) {        
        return NULL;
    }
    
    // Verificar token '>'
    if (!match_token(TOKEN_MAYOR)) {        
        return NULL;
    }
    consume_token(); // Consumir '>'
    
    // Verificar fin de instrucción
    if (!match_token(TOKEN_FINLINEA)) {        
        return NULL;
    }
    consume_token(); // Consumir ':_:'
    
    // Crear nodo de salida
    ASTNode* output_node = create_node(NODE_OUTPUT, mt_token.lexema, mt_token.linea);
    add_child(output_node, expr_node);
    
    return output_node;
}

/**
 * Parsear término
 * Termino -> Factor ((*|/) Factor)*
 */
ASTNode* parse_term_tree() {
    ASTNode* left = parse_factor_tree();
    
    while (match_token(TOKEN_POR) || match_token(TOKEN_DIV)) {
        Token op = consume_token();
        ASTNode* right = parse_factor_tree();
        left = create_binary_node(NODE_BINARY_OP, left, right);
        strcpy(left->value, op.lexema);
    }
    
    return left;
}

/**
 * Parsear factor
 * Factor -> Numero | Cadena | Caracter | Booleano | Identificador
 */
ASTNode* parse_factor_tree() {
    Token token = peek_token();
    
    switch (token.type) {
        case TOKEN_NUMERO:
            consume_token();
            return create_node(NODE_NUMBER, token.lexema, token.linea);
            
        case TOKEN_CONSTANTE_CADENA:
            consume_token();
            return create_node(NODE_STRING, token.lexema, token.linea);
            
        case TOKEN_CONSTANTE_CHAR:
            consume_token();
            return create_node(NODE_CHAR, token.lexema, token.linea);
            
        case TOKEN_CONSTANTE_BOOL:
            consume_token();
            return create_node(NODE_BOOLEAN, token.lexema, token.linea);
            
        case TOKEN_IDENTIFICADOR:
            consume_token();
            return create_node(NODE_IDENTIFIER, token.lexema, token.linea);
            
        default:
            /*printf("Error: factor inesperado '%s' en línea %d\n", 
                   token.lexema, token.linea);*/
            return NULL;
    }
}