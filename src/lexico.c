#include "lexico.h"

// Definición de variables globales
Token tokens[MAX_TOKENS];
int token_count = 0;
int current_line = 1;

// Variables para mantener estado entre líneas
static bool es_declaracion_global = false;
static char ultimo_token[MAX_LINE_LENGTH] = "";
static FILE *output_file = NULL;

void set_output_file(FILE *file) {
    output_file = file;
}

const char* get_token_name(TokenType type) {
    switch(type) {
        case TOKEN_IDENTIFICADOR: return "IDENTIFICADOR";
        case TOKEN_NUMERO: return "CONSTANTE_NUMERO";
        case TOKEN_FINLINEA: return "FINLINEA";
        case TOKEN_ARROBA: return "ARROBA";
        case TOKEN_COMILLA: return "COMILLA";
        case TOKEN_COMA: return "COMA";
        case TOKEN_IGUAL: return "IGUAL";
        case TOKEN_MAS: return "MAS";
        case TOKEN_MENOS: return "MENOS";
        case TOKEN_POR: return "POR";
        case TOKEN_DIV: return "DIV";
        case TOKEN_MENOR: return "MENOR";
        case TOKEN_MAYOR: return "MAYOR";
        case TOKEN_SALIDA: return "SALIDA";
        case TOKEN_ENTRADA: return "ENTRADA";
        case TOKEN_ENTERO: return "ENTERO";
        case TOKEN_DECIMAL: return "DECIMAL";
        case TOKEN_CARACTER: return "CARACTER";
        case TOKEN_CADENA: return "CADENA";
        case TOKEN_BOOLEANO: return "BOOLEANO";
        case TOKEN_CONSTANTE_CADENA: return "CONSTANTE_CADENA";
        case TOKEN_CONSTANTE_CHAR: return "CONSTANTE_CHAR";
        case TOKEN_CONSTANTE_BOOL: return "CONSTANTE_BOOL";
        case TOKEN_EOF: return "EOF";
        case TOKEN_DESCONOCIDO: return "ERROR";
        default: return "DESCONOCIDO";
    }
}

// Implementar todas las demás funciones del análisis léxico aquí...
// (Copiar las funciones correspondientes del archivo original)

/**
 * Función para realizar el análisis léxico completo
 * @param source_file: Es el código fuente a analizar
 */
void analisis_lexico(FILE *source_file) {
    char line[MAX_LINE_LENGTH];
    int line_number = 1;
    
    // Leer línea por línea
    while (fgets(line, sizeof(line), source_file)) {
        char token[MAX_LINE_LENGTH];
        int pos = 0;
        int line_len = strlen(line);
        
        // Procesar todos los tokens de la línea actual
        while (pos < line_len) {
            // Saltar espacios en blanco
            while (pos < line_len && (line[pos] == ' ' || line[pos] == '\t')) pos++;
            
            // Si llegamos al final de la línea o encontramos salto de línea, salir
            if (pos >= line_len || line[pos] == '\n' || line[pos] == '\0') break;
            
            int i = 0;
            int start_pos = pos; // Guardar posición inicial para detectar si no avanzamos
            
            // Manejo del fin de instrucción :_:
            if (pos + 2 < line_len && line[pos] == ':' && line[pos+1] == '_' && line[pos+2] == ':') {
                strcpy(token, ":_:");
                pos += 3;
                agregar_token(token, line_number);
                continue;
            }
            
            // Manejo de constantes de carácter 'x'
            if (line[pos] == '\'') {
                token[i++] = line[pos++];
                if (pos < line_len && line[pos] != '\'' && line[pos] != '\n') {
                    token[i++] = line[pos++];
                }
                if (pos < line_len && line[pos] == '\'') {
                    token[i++] = line[pos++];
                }
                token[i] = '\0';
                agregar_token(token, line_number);
                continue;
            }
            
            // REEMPLAZA esta parte en analisis_lexico():

            // Manejo de cadenas @texto@
            if (line[pos] == '@') {
                token[i++] = line[pos++];
                
                // CORRECCIÓN: Leer todo el contenido hasta encontrar el @ de cierre
                // INCLUYENDO espacios, números, letras, etc.
                while (pos < line_len && line[pos] != '@' && line[pos] != '\n' && line[pos] != '\0') {
                    token[i++] = line[pos++];
                }
                
                // Si encontramos el @ de cierre, agregarlo
                if (pos < line_len && line[pos] == '@') {
                    token[i++] = line[pos++];
                }
                
                token[i] = '\0';
                agregar_token(token, line_number);
                continue;
            }
            
            // Operadores de un solo carácter
            if (strchr("=<>+-*/,", line[pos]) != NULL) {
                token[0] = line[pos];
                token[1] = '\0';
                pos++;
                agregar_token(token, line_number);
                continue;
            }
            
            // Identificadores, números y palabras reservadas
            while (pos < line_len && line[pos] != ' ' && line[pos] != '\t' && 
                   line[pos] != '\n' && line[pos] != '\0' && 
                   strchr("=<>+-*/,:@'", line[pos]) == NULL) {
                token[i++] = line[pos++];
            }
            
            if (i > 0) {
                token[i] = '\0';
                agregar_token(token, line_number);
            }
            
            // CRÍTICO: Verificar que hemos avanzado en la posición
            if (pos == start_pos) {
                // Si no avanzamos, saltar el carácter problemático para evitar bucle infinito
                pos++;
            }
        }
        
        // IMPORTANTE: Incrementar número de línea después de procesar toda la línea
        line_number++;
    }
}

/**
 * Función para agregar un token a la tabla de tokens
 * @param lexema: Lexema al que pertence el token, linea donde se encuentra el token
 */
void agregar_token(const char *lexema, int linea) {
    if (token_count >= MAX_TOKENS) return;
    
    TokenType type = clasificar_token(lexema);
    tokens[token_count].type = type;
    strcpy(tokens[token_count].lexema, lexema);
    tokens[token_count].linea = linea;
    token_count++;
}


/**
 * Función para clasificar un lexema y determinar su tipo de token
 */
TokenType clasificar_token(const char *lexema) {
    // Palabras reservadas
    if (strcmp(lexema, "entero") == 0) return TOKEN_ENTERO;
    if (strcmp(lexema, "decimal") == 0) return TOKEN_DECIMAL;
    if (strcmp(lexema, "caracter") == 0) return TOKEN_CARACTER;
    if (strcmp(lexema, "cadena") == 0) return TOKEN_CADENA;
    if (strcmp(lexema, "booleano") == 0) return TOKEN_BOOLEANO;
    if (strcmp(lexema, "mt") == 0) return TOKEN_SALIDA;
    if (strcmp(lexema, "inp") == 0) return TOKEN_ENTRADA;
    if (strcmp(lexema, "true") == 0 || strcmp(lexema, "false") == 0) return TOKEN_CONSTANTE_BOOL;
    
    // Operadores y símbolos
    if (strcmp(lexema, ":_:") == 0) return TOKEN_FINLINEA;
    if (strcmp(lexema, "@") == 0) return TOKEN_ARROBA;
    if (strcmp(lexema, "'") == 0) return TOKEN_COMILLA;
    if (strcmp(lexema, ",") == 0) return TOKEN_COMA;
    if (strcmp(lexema, "=") == 0) return TOKEN_IGUAL;
    if (strcmp(lexema, "+") == 0) return TOKEN_MAS;
    if (strcmp(lexema, "-") == 0) return TOKEN_MENOS;
    if (strcmp(lexema, "*") == 0) return TOKEN_POR;
    if (strcmp(lexema, "/") == 0) return TOKEN_DIV;
    if (strcmp(lexema, "<") == 0) return TOKEN_MENOR;
    if (strcmp(lexema, ">") == 0) return TOKEN_MAYOR;
    

    // Constantes de carácter ('a')
    if (strlen(lexema) == 3 && lexema[0] == '\'' && lexema[2] == '\'') {
        return TOKEN_CONSTANTE_CHAR;
    }
    
    // Constantes de cadena (@texto@)
    if (strlen(lexema) >= 2 && lexema[0] == '@' && lexema[strlen(lexema)-1] == '@') {
        return TOKEN_CONSTANTE_CADENA;
    }
    
    // Números
    if (es_numero(lexema)) return TOKEN_NUMERO;
    
    // Identificadores
    if (es_identificador_valido(lexema)) return TOKEN_IDENTIFICADOR;
    
    return TOKEN_DESCONOCIDO;
}

/**
 * Función para extraer los tokens siguientes de la entrada
 * @param line: Para la línea de código fuente
 * @param pos: Para la posición actual en la línea
 * @param token: Token extraído de la línea de código
 */
int obtener_siguiente_token(const char *line, int pos, char *token) {
    int i = 0;
    // Para saltar (evitar) espacios en blanco o tabulaciones
    while (line[pos] == ' ' || line[pos] == '\t') pos++;
    if (line[pos] == '\0' || line[pos] == '\n') return pos;

    // Manejo para el fin de instrucción :_: de infinix
    if (line[pos] == ':' && line[pos+1] == '_' && line[pos+2] == ':') {
        strcpy(token, ":_:");
        return pos + 3;
    }

    // Manejo de constantes de carácter (entre comillas simples)
    if (line[pos] == '\'') {
        token[i++] = line[pos++]; // Agregar la comilla inicial
        
        // Leer el carácter (debe haber exactamente uno)
        if (line[pos] != '\0' && line[pos] != '\n' && line[pos] != '\'') {
            token[i++] = line[pos++];
        }
        
        // Buscar la comilla de cierre
        if (line[pos] == '\'') {
            token[i++] = line[pos++]; // Agregar la comilla final
        }
        
        token[i] = '\0';
        return pos;
    }

    // CORRECCIÓN: Manejo mejorado de cadenas con @
    if (line[pos] == '@') {
        token[i++] = line[pos++]; // Agregar el @ inicial
        
        // Buscar el @ de cierre, INCLUYENDO espacios en el contenido
        while (line[pos] != '@' && line[pos] != '\0' && line[pos] != '\n') {
            token[i++] = line[pos++]; // Incluir TODO el contenido, incluso espacios
        }
        
        // Si encontramos el @ de cierre, agregarlo
        if (line[pos] == '@') {
            token[i++] = line[pos++]; // Agregar el @ final
        }
        
        token[i] = '\0';
        return pos;
    }

    // Manejo de operadores de un solo carácter
    if (strchr("=<>+-*/?:@,", line[pos]) != NULL) {
        token[i++] = line[pos++];
        token[i] = '\0';
        return pos;
    }

    // Lectura de identificadores y números (caracteres alfanúmericos)
    while (line[pos] != ' ' && line[pos] != '\t' && line[pos] != '\0' &&
           line[pos] != '\n' && strchr("=<>+-*/@:,'", line[pos]) == NULL) {
        token[i++] = line[pos++];
    }
    token[i] = '\0';
    return pos;
}

/**
 * Función para imprimir la tabla de tokens (análisis léxico)
 */
void imprimir_tabla_tokens() {
    printf("\n=== ANÁLISIS LÉXICO ===\n");
    printf("%-6s %-20s %-s\n", "Línea", "Tipo", "Lexema");
    printf("%-6s %-20s %-s\n", "------", "--------------------", "----------");
    
    for (int i = 0; i < token_count; i++) {
        printf("%-6d %-20s %s\n", 
               tokens[i].linea, 
               get_token_name(tokens[i].type), 
               tokens[i].lexema);
    }
}

/**
 * Función para verificar si una línea contiene solo espacios en blanco
 * @param line: La línea a verificar
 * @return: true si la línea está vacía o solo contiene espacios, false en caso contrario
 */
bool es_linea_vacia(const char *line) {
    for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            return false;
        }
    }
    return true;
}

void procesar_linea_codigo(const char *line) {
    char token[MAX_LINE_LENGTH]; // Longitud de línea
    int pos = 0; // Posición
    int line_len = strlen(line); // Longitud de la línea
    
    // Si la línea está vacía o solo contiene espacios, no procesarla
    if (es_linea_vacia(line)) {
        return;
    }

    // Procesamiento de la línea token por token
    while (pos < line_len) {
        int old_pos = pos;  // Para evitar ciclos, se guarda la posición anterior
        // Obteneción del siguiente token
        pos = obtener_siguiente_token(line, pos, token);

        // Si no se avanza en la línea de código o no hay contenido del token se rompe el bucle
        if (pos == old_pos || strlen(token) == 0) break;

        // Evitar duplicación de tokens: si el token actual es igual al último token procesado
        // y es un operador o símbolo, no lo procesamos nuevamente
        if (strlen(ultimo_token) > 0 && strcmp(token, ultimo_token) == 0) {
            // Solo evitar duplicación para operadores y símbolos específicos
            if (strcmp(token, ",") == 0 || strcmp(token, "=") == 0 ||
                strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
                strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
                strcmp(token, "<") == 0 || strcmp(token, ">") == 0 ||
                strcmp(token, "@") == 0) {
                continue; // Saltar este token duplicado
            }
        }

        // Detección de tipos de datos, indica una declaración de variable para actualizar la variable es_declaracion_global
        if (strcmp(token, "entero") == 0 || strcmp(token, "decimal") == 0 ||
            strcmp(token, "caracter") == 0 || strcmp(token, "cadena") == 0 ||
            strcmp(token, "booleano") == 0) {
            es_declaracion_global = true;
        }

        // Para asignar la clasificación del token actual
        SymbolType type = clasificacion_tokens(token, es_declaracion_global && !es_palabra_reservada(token));
        // Escritura del token en el archivo .obj
        escribir_contenido_obj(token, type);

        // Guardar el token actual como último token procesado
        strcpy(ultimo_token, token);

        // Cuando se detecta un fin de instrucción se termina la declaración de variables (en caso que lo sea). Se reinicia la variable y el procesamiento de la línea
        if (strcmp(token, ":_:") == 0) {
            es_declaracion_global = false;
            strcpy(ultimo_token, ""); // Limpiar el último token al final de la instrucción
            break;
        }
    }
}

/**
 * Función para escribir el contenido del token al archivo objeto según su clasificación (tipo) de símbolo
 * @param token: Para el token a escribir
 * @param type: Para el tipo de token
 */
void escribir_contenido_obj(const char *token, SymbolType type) {
    switch(type) {
        case INTERNAL_SYMBOL:
            // Para palabras reservadas y operadores se escriben tal como son
            fprintf(output_file, "%d %s\n", current_line, token);
            break;
            
        case EXTERNAL_VARIABLE:
            fprintf(output_file, "%d var\n", current_line);
            break;
            
        case CONSTANT_STRING:
            // Manejar cadenas completas @texto@
            if (token[0] == '@' && token[strlen(token) - 1] == '@' && strlen(token) > 2) {
                fprintf(output_file, "%d @\n", current_line);
                fprintf(output_file, "%d const_cadena\n", current_line);
                fprintf(output_file, "%d @\n", current_line);
            } 
            // Manejar @ individual
            else if (strcmp(token, "@") == 0) {
                fprintf(output_file, "%d @\n", current_line);
            }
            // Contenido de cadena sin @ (texto entre @texto@)
            else {
                fprintf(output_file, "%d const_cadena\n", current_line);
            }
            break;
            
        case CONSTANT_CHAR:
            // Para constantes de carácter 'x'
            if (token[0] == '\'' && token[strlen(token) - 1] == '\'' && strlen(token) == 3) {
                fprintf(output_file, "%d '\n", current_line);
                fprintf(output_file, "%d const_caracter\n", current_line);
                fprintf(output_file, "%d '\n", current_line);
            }
            // Solo comilla simple
            else if (strcmp(token, "'") == 0) {
                fprintf(output_file, "%d '\n", current_line);
            }
            break;
            
        case CONSTANT_NUMBER:
            fprintf(output_file, "%d const_numerica\n", current_line);
            break;
            
        case CONSTANT_BOOLEAN:
            fprintf(output_file, "%d const_booleana\n", current_line);
            break;
    }
}