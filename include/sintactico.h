#ifndef SINTACTICO_H
#define SINTACTICO_H

#include "tipos.h"
#include "lexico.h"

// Variables globales del análisis sintáctico
extern int current_token;
extern bool syntax_error;
extern char error_message[256];

// Funciones principales
bool analisis_sintactico(void);
bool parse_programa(void);
bool parse_instruccion(void);

// Funciones de parsing específicas
bool parse_declaracion(void);
bool parse_tipo_dato(void);
bool parse_lista_declaraciones(void);
bool parse_declaracion_simple(void);
bool parse_asignacion(void);
bool parse_instruccion_salida(void);
bool parse_instruccion_entrada(void);
bool parse_lista_elementos_salida(void);
bool parse_elemento_salida(void);
bool parse_expresion(void);
bool parse_termino(void);
bool parse_factor(void);
bool parse_cadena_literal(void);

// Funciones de utilidad
Token* get_current_token(void);
void next_token(void);
bool match(TokenType expected_type);
void syntax_error_msg(const char* expected);
void panic_mode_sync(void);
bool is_valid_instruction_start(void);

#endif // SINTACTICO_H