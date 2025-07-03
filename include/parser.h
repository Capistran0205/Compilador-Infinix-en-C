// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "tipos.h"
#include "ast.h"
#include "lexico.h"

// Variables globales del parser
extern int current_token_index;
extern Token current_token_tree;

// Funciones del parser
ASTNode* parse_program_tree();
ASTNode* parse_statement_tree();
ASTNode* parse_declaration_tree();
ASTNode* parse_assignment_tree();
ASTNode* parse_expression_tree();
ASTNode* parse_factor_tree(); 
ASTNode* parse_input_tree();
ASTNode* parse_output_tree();
ASTNode* parse_term_tree() ;

// Utilidades
Token peek_token();
Token consume_token();
bool match_token(TokenType expected);
bool is_type_token(TokenType type);

#endif
