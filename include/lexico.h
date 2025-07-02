#ifndef LEXICO_H
#define LEXICO_H

#include "tipos.h"
#include "tabla_simbolos.h"

// Variables globales del análisis léxico
extern Token tokens[MAX_TOKENS];
extern int token_count;
extern int current_line;

// Funciones del análisis léxico
void analisis_lexico(FILE *source_file);
TokenType clasificar_token(const char *lexema);
void agregar_token(const char *lexema, int linea);
int obtener_siguiente_token(const char *line, int pos, char *token);
const char* get_token_name(TokenType type);
void imprimir_tabla_tokens(void);

// Funciones para procesamiento de líneas
bool es_linea_vacia(const char *line);
void procesar_linea_codigo(const char *line);
void escribir_contenido_obj(const char *token, SymbolType type);
void set_output_file(FILE *file);

#endif // LEXICO_H