#ifndef TIPOS_H
#define TIPOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Constantes globales
#define MAX_SYMBOLS 100
#define MAX_IDENTIFIERS 100
#define MAX_LINE_LENGTH 256
#define MAX_TOKENS 1000

// Enumeración de tipos de tokens
typedef enum {
    TOKEN_IDENTIFICADOR,
    TOKEN_NUMERO,
    TOKEN_FINLINEA,         // :_:
    TOKEN_ARROBA,           // @
    TOKEN_COMILLA,          // '
    TOKEN_COMA,             // ,
    TOKEN_IGUAL,            // =
    TOKEN_MAS,              // +
    TOKEN_MENOS,            // -
    TOKEN_POR,              // *
    TOKEN_DIV,              // /
    TOKEN_MENOR,            // <
    TOKEN_MAYOR,            // >
    TOKEN_SALIDA,           // mt
    TOKEN_ENTRADA,          // inp
    TOKEN_ENTERO,           // entero
    TOKEN_DECIMAL,          // decimal
    TOKEN_CARACTER,         // caracter
    TOKEN_CADENA,           // cadena
    TOKEN_BOOLEANO,         // booleano
    TOKEN_CONSTANTE_CADENA, // @texto@
    TOKEN_CONSTANTE_CHAR,   // 'a'
    TOKEN_CONSTANTE_BOOL,   // true/false
    TOKEN_EOF,              // Fin de archivo
    TOKEN_DESCONOCIDO,      // Token no reconocido
    TOKEN_SYNC_POINT        // Token para puntos de sincronización
} TokenType;

// Estructura del token
typedef struct {
    TokenType type;
    char lexema[MAX_LINE_LENGTH];
    int linea;
} Token;

// Tipos de datos
typedef enum {
    TYPE_ENTERO,
    TYPE_DECIMAL, 
    TYPE_CARACTER,
    TYPE_CADENA,
    TYPE_BOOLEANO,
    TYPE_UNKNOWN
} DataType;

// Tipos de símbolos
typedef enum {
    INTERNAL_SYMBOL,
    EXTERNAL_VARIABLE,
    CONSTANT_STRING,
    CONSTANT_CHAR,
    CONSTANT_NUMBER,
    CONSTANT_BOOLEAN
} SymbolType;

// Entrada de símbolo
typedef struct {
    char name[50];
    SymbolType type;
    DataType data_type;
    bool is_initialized;
    int scope_level;
} SymbolEntry;

#endif // TIPOS_H