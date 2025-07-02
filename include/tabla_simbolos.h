#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include "tipos.h"

// Variables globales de tablas
extern SymbolEntry internal_symbols[MAX_SYMBOLS];
extern SymbolEntry external_symbols[MAX_IDENTIFIERS];
extern int internal_count;
extern int external_count;

// Funciones de tabla de símbolos
void inicializar_tabla_interna_simbolos(void);
bool es_palabra_reservada(const char *name);
bool es_variable(const char *name);
void agregar_variable(const char *name);
void imprimir_tabla_simbolos(void);

// Funciones de validación de tokens
bool es_constante_booleana(const char *token);
bool es_constante_caracter(const char *token);
bool es_numero(const char *token);
bool es_identificador_valido(const char *str);
SymbolType clasificacion_tokens(const char *token, bool es_declaracion);

#endif // TABLA_SIMBOLOS_H