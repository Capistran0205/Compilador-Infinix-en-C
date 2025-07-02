#include "tabla_simbolos.h"

// Definición de variables globales
SymbolEntry internal_symbols[MAX_SYMBOLS];
SymbolEntry external_symbols[MAX_IDENTIFIERS];
int internal_count = 0;
int external_count = 0;

void inicializar_tabla_interna_simbolos() {
    char *keywords[] = {"entero", "decimal", "caracter", "cadena", "booleano", "inp", "mt", "true", "false"};
    for (size_t i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, keywords[i]);
    }

    char *symbols[] = {"<", ">", ":_:", "=", "+", "-", "*", "/", "@", ",", "'"};
    for (size_t i = 0; i < sizeof(symbols)/sizeof(symbols[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, symbols[i]);
    }

    for (int i = 0; i < internal_count; i++) {
        internal_symbols[i].type = INTERNAL_SYMBOL;
    }
}

bool es_palabra_reservada(const char *name) {
    for (int i = 0; i < internal_count; i++) {
        if (strcmp(internal_symbols[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

bool es_variable(const char *name) {
    for (int i = 0; i < external_count; i++) {
        if (strcmp(external_symbols[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

void agregar_variable(const char *name) {
    if (external_count < MAX_IDENTIFIERS && !es_variable(name)) {
        strcpy(external_symbols[external_count].name, name);
        external_symbols[external_count].type = EXTERNAL_VARIABLE;
        external_count++;
    }
}

bool es_constante_booleana(const char *token) {
    return (strcmp(token, "true") == 0 || strcmp(token, "false") == 0);
}

bool es_constante_caracter(const char *token) {
    int len = strlen(token);
    if (len != 3) return false;
    if (token[0] != '\'' || token[2] != '\'') return false;
    if (token[1] == '\0') return false;
    return true;
}

bool es_numero(const char *token) {
    if (strlen(token) == 0) return false;
    char *endptr;
    strtod(token, &endptr);
    return *endptr == '\0';
}

bool es_identificador_valido(const char *str) {
    if (strlen(str) == 0) return false;
    if (!isalpha(str[0]) && str[0] != '_') return false;
    for (int i = 1; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && str[i] != '_') return false;
    }
    return true;
}

SymbolType clasificacion_tokens(const char *token, bool es_declaracion) {
    if (es_constante_booleana(token)) return CONSTANT_BOOLEAN;
    if (es_constante_caracter(token)) return CONSTANT_CHAR;
    if (es_palabra_reservada(token) && !es_constante_booleana(token)) return INTERNAL_SYMBOL;
    if (es_numero(token)) return CONSTANT_NUMBER;
    if (token[0] == '@') return CONSTANT_STRING;
    
    if (es_declaracion && !es_palabra_reservada(token)) {
        // printf("Variable agregada %s a tabla externa\n", token);
        agregar_variable(token);
        return EXTERNAL_VARIABLE;
    }
    
    if (es_variable(token)) return EXTERNAL_VARIABLE;
    return CONSTANT_STRING;
}

void imprimir_tabla_simbolos() {
    printf("\n=== TABLA INTERNA ===\n");
    for (int i = 0; i < internal_count; i++) {
        printf("%s\n", internal_symbols[i].name);
    }
    printf("\n=== TABLA EXTERNA ===\n");
    for (int i = 0; i < external_count; i++) {
        printf("%s\n", external_symbols[i].name);
    }
}