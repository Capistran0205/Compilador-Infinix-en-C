#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_SYMBOLS 100
#define MAX_IDENTIFIERS 100
#define MAX_LINE_LENGTH 256

typedef enum {
    INTERNAL_SYMBOL,
    EXTERNAL_VARIABLE,
    CONSTANT_STRING,
    CONSTANT_NUMBER
} SymbolType;

typedef struct {
    char name[50];
    SymbolType type;
} SymbolEntry;

SymbolEntry internal_symbols[MAX_SYMBOLS];
SymbolEntry external_symbols[MAX_IDENTIFIERS];
int internal_count = 0;
int external_count = 0;
int current_line = 1;
FILE *output_file;

void inicializar_tabla_interna_simbolos() {
    char *keywords[] = {"entero", "decimal", "caracter", "cadena", "booleano", "inp", "mt"};
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, keywords[i]);
    }

    char *symbols[] = {"<", ">", ":_:", "=", "+", "-", "*", "/", "(", ")", "@", ","};
    for (int i = 0; i < sizeof(symbols)/sizeof(symbols[0]); i++) {
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

bool es_numero(const char *token) {
    if (strlen(token) == 0) return false;
    char *endptr;
    strtod(token, &endptr);
    return *endptr == '\0';
}

SymbolType clasificacion_tokens(const char *token, bool is_declaration) {
    if (es_palabra_reservada(token)) return INTERNAL_SYMBOL;
    if (es_numero(token)) return CONSTANT_NUMBER;
    if (token[0] == '@') return CONSTANT_STRING;
    if (is_declaration && !es_palabra_reservada(token)) {
        agregar_variable(token);
        return EXTERNAL_VARIABLE;
    }
    if (es_variable(token)) return EXTERNAL_VARIABLE;
    return CONSTANT_STRING;
}

int obtener_siguiente_token(const char *line, int pos, char *token) {
    int i = 0;
    while (line[pos] == ' ' || line[pos] == '\t') pos++;
    if (line[pos] == '\0' || line[pos] == '\n') return pos;

    if (line[pos] == ':' && line[pos+1] == '_' && line[pos+2] == ':') {
        strcpy(token, ":_:");
        return pos + 3;
    }

    if (line[pos] == '@') {
        token[i++] = line[pos++];
        if (line[pos] == ' ' || line[pos] == '\t' || line[pos] == '\0' || line[pos] == '\n' ||
            line[pos] == '>' || line[pos] == '<') {
            token[i] = '\0';
            return pos;
        }
        while (line[pos] != '@' && line[pos] != '\0' && line[pos] != '\n') {
            token[i++] = line[pos++];
        }
        if (line[pos] == '@') {
            token[i++] = line[pos++];
        }
        token[i] = '\0';
        return pos;
    }

    if (strchr("=<>+-*/(),", line[pos]) != NULL) {
        token[i++] = line[pos++];
        token[i] = '\0';
        return pos;
    }

    while (line[pos] != ' ' && line[pos] != '\t' && line[pos] != '\0' &&
           line[pos] != '\n' && strchr("=<>+-*/()@:,", line[pos]) == NULL) {
        token[i++] = line[pos++];
    }
    token[i] = '\0';
    return pos;
}

void escribir_contenido_obj(const char *token, SymbolType type) {
    switch(type) {
        case INTERNAL_SYMBOL:
            fprintf(output_file, "%d %s\n", current_line, token);
            break;
        case EXTERNAL_VARIABLE:
            fprintf(output_file, "%d var\n", current_line);
            break;
        case CONSTANT_STRING:
            if (token[0] == '@' && token[strlen(token) - 1] == '@' && strlen(token) > 2) {
                fprintf(output_file, "%d @\n", current_line);
                fprintf(output_file, "%d const_cadena\n", current_line);
                fprintf(output_file, "%d @\n", current_line);
            } else if (strcmp(token, "@") == 0) {
                fprintf(output_file, "%d @\n", current_line);
            } else {
                fprintf(output_file, "%d const_cadena\n", current_line);
            }
            break;
        case CONSTANT_NUMBER:
            fprintf(output_file, "%d const_numerica\n", current_line);
            break;
    }
}

void procesar_linea_codigo(const char *line) {
    char token[256];
    int pos = 0;
    bool in_declaration = false;
    int line_len = strlen(line);

    while (pos < line_len) {
        int old_pos = pos;  // Para evitar ciclos
        pos = obtener_siguiente_token(line, pos, token);

        if (pos == old_pos || strlen(token) == 0) break;

        if (strcmp(token, "entero") == 0 || strcmp(token, "decimal") == 0 ||
            strcmp(token, "caracter") == 0 || strcmp(token, "cadena") == 0 ||
            strcmp(token, "booleano") == 0) {
            in_declaration = true;
        }

        SymbolType type = clasificacion_tokens(token, in_declaration && !es_palabra_reservada(token));
        escribir_contenido_obj(token, type);

        if (strcmp(token, ":_:") == 0) {
            in_declaration = false;
            break;
        }
    }
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

int main() {
    inicializar_tabla_interna_simbolos();

    FILE *source_file = fopen("codigo_fuente.txt", "r");
    if (source_file == NULL) {
        printf("Error: No se pudo abrir el archivo de entrada\n");
        return 1;
    }

    output_file = fopen("codigo_objeto.obj", "w");
    if (output_file == NULL) {
        printf("Error: No se pudo crear el archivo de salida\n");
        fclose(source_file);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    current_line = 1;

    while (fgets(line, sizeof(line), source_file)) {
        if (strlen(line) > 0) {
            procesar_linea_codigo(line);
            current_line++;
        }
    }

    imprimir_tabla_simbolos();

    fclose(source_file);
    fclose(output_file);

    printf("Procesamiento completado correctamente.\n");
    return 0;
}