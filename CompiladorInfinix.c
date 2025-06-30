#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Valores constantes
#define MAX_SYMBOLS 100
#define MAX_IDENTIFIERS 100
#define MAX_LINE_LENGTH 256
#define MAX_TOKENS 1000

// Enumeración de tipos de tokens para el análisis léxico
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
    TOKEN_SALIDA,            // mt
    TOKEN_ENTRADA,              // inp
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

// Estructura para representar un token
typedef struct {
    TokenType type;
    char lexema[MAX_LINE_LENGTH];
    int linea;
} Token;

// Enumeración de tipos de símbolos (tu código original)
typedef enum {
    INTERNAL_SYMBOL,
    EXTERNAL_VARIABLE,
    CONSTANT_STRING,
    CONSTANT_CHAR,
    CONSTANT_NUMBER,
    CONSTANT_BOOLEAN
} SymbolType;

// Estructura de entrada de símbolos (tu código original)
typedef struct {
    char name[50];
    SymbolType type;
} SymbolEntry;

// Variables globales
SymbolEntry internal_symbols[MAX_SYMBOLS];
SymbolEntry external_symbols[MAX_IDENTIFIERS];
int internal_count = 0;
int external_count = 0;
int current_line = 1;
FILE *output_file;

// Variables para el análisis léxico
Token tokens[MAX_TOKENS];
int token_count = 0;
int current_token = 0;
TokenType sync_tokens[] = {
    TOKEN_FINLINEA,    // :_: (fin de instrucción)
    TOKEN_ENTERO,      // Inicio de nueva declaración
    TOKEN_DECIMAL,
    TOKEN_CARACTER,
    TOKEN_CADENA, 
    TOKEN_BOOLEANO,
    TOKEN_SALIDA,      // mt (nueva instrucción)
    TOKEN_ENTRADA,     // inp (nueva instrucción)
    TOKEN_EOF
};

// Variables para el análisis sintáctico
bool syntax_error = false;
char error_message[256];

/**
 * Función para obtener el nombre del tipo de token
 */
const char* get_token_name(TokenType type) {
    switch(type) {
        case TOKEN_IDENTIFICADOR: return "IDENTIFICADOR";
        case TOKEN_NUMERO: return "NUMERO";
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

/**
 * Función para inicializar la tabla interna de símbolos
 */
void inicializar_tabla_interna_simbolos() {
    char *keywords[] = {"entero", "decimal", "caracter", "cadena", "booleano", "inp", "mt", "true", "false"};
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, keywords[i]);
    }

    char *symbols[] = {"<", ">", ":_:", "=", "+", "-", "*", "/", "@", ",", "'"};
    for (int i = 0; i < sizeof(symbols)/sizeof(symbols[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, symbols[i]);
    }

    for (int i = 0; i < internal_count; i++) {
        internal_symbols[i].type = INTERNAL_SYMBOL;
    }
}

/**
 * Función para verificar si un token, su descripción (parámetro de puntero char definido) pertenece o no a una palabra reservada del lenguaje
 * @param name: Token a verificar
 * @return: Devuelve true si coíncide con una palabra reservada del lenguaje y false en caso contrario
 */
bool es_palabra_reservada(const char *name) {
    for (int i = 0; i < internal_count; i++) {
        // Comprobar si la palabra corresponde a un palabra reservada del lenguaje infinix
        if (strcmp(internal_symbols[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Función para verificar si un token corresponde a una constante booleana (true o false)
 * @param token: Token a verificar
 * @return: Devuelve true si es una constante booleana y false en caso contrario
 */
bool es_constante_booleana(const char *token) {
    // Verificar si el token es exactamente "true" o "false"
    return (strcmp(token, "true") == 0 || strcmp(token, "false") == 0);
}

/**
 * Función para verificar si un token corresponde a una constante de carácter
 * Una constante de carácter en Infinix debe estar entre comillas simples: 'a', 'Z', '1', etc.
 * @param token: Token a verificar
 * @return: Devuelve true si es una constante de carácter válida y false en caso contrario
 */
bool es_constante_caracter(const char *token) {
    int len = strlen(token);
    
    // Una constante de carácter debe tener exactamente 3 caracteres: 'x'
    if (len != 3) {
        return false;
    }
    
    // Debe comenzar y terminar con comilla simple
    if (token[0] != '\'' || token[2] != '\'') {
        return false;
    }
    
    // El carácter del medio debe ser un carácter válido (no nulo)
    if (token[1] == '\0') {
        return false;
    }
    
    return true;
}

/**
 * Función para verificar si un token, su descripción (parámetro de puntero char definido) pertenece o no a un variable declarada
 * @param name: Token a verificar
 * @return: Devuelve true si ya existe (variable declarada) y false de que no exista
 */
bool es_variable(const char *name) {
    for (int i = 0; i < external_count; i++) {
        // Comprobar si la palabra reservada corresponde a una variable no declarada (no guardada) en la tabla externa
        if (strcmp(external_symbols[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Función para añadir el token (parámetro de puntero char definido) como una nueva variable a la tabla de símbolos externa, no devuelve nada
 * @param name: El nombre de la variable
 */
void agregar_variable(const char *name) {
    // Validar que no se pase el límite de variables y que dicha variable no se encuentre ya declarada (agregada a la tabla externa) previamente
    if (external_count < MAX_IDENTIFIERS && !es_variable(name)) {
        strcpy(external_symbols[external_count].name, name); // Copia el valor del token al arreglo de la tabla externa (variables)
        external_symbols[external_count].type = EXTERNAL_VARIABLE; // Se clasifica como variable
        external_count++; // Incrementa el contador de variables
    }
}

/**
 * Función para verificar si el token corresponde (parámetro de puntero char definido) a una constante númerica ya sea entera o flotante
 * @param token: Token a verificar
 * @return:Devuelve true si es una cosntante númerica y false en caso contrario
 */
bool es_numero(const char *token) {
    if (strlen(token) == 0) return false; // NO hay contenido, entonces false
    char *endptr;
    strtod(token, &endptr); // Conversión a double, el puntero endptr apunta la primer carácter del parámetro token
    return *endptr == '\0'; // Si endptr apunta (llega) al final de la cadena todo el string es un número (entero o decimal)
}

/**
 * Función para clasificar los tokens según su tipo (versión mejorada con soporte para constantes de carácter y booleanas)
 * @param token: Token a clasificar
 * @param es_declaracion: true si es una declaración de variable, false si es palabra reservada o constante
 * @return: Devuelve el tipo de token
 */
SymbolType clasificacion_tokens(const char *token, bool es_declaracion) {
    // Validación aplicando las funciones previamente definidas
    
    // Primero verificar constantes booleanas (true/false)
    if (es_constante_booleana(token)) return CONSTANT_BOOLEAN;
    
    // Verificar constantes de carácter ('a', 'Z', etc.)
    if (es_constante_caracter(token)) return CONSTANT_CHAR;
    
    // Verificar palabras reservadas (excluyendo true/false que ya se procesaron como constantes)
    if (es_palabra_reservada(token) && !es_constante_booleana(token)) return INTERNAL_SYMBOL;
    
    // Verificar constantes numéricas
    if (es_numero(token)) return CONSTANT_NUMBER;
    
    // Verificar constantes de cadena (inician con @)
    if (token[0] == '@') return CONSTANT_STRING;
    
    // Se añade la variable si es una declaración y no es palabra reservada
    if (es_declaracion && !es_palabra_reservada(token)) {
        agregar_variable(token);
        return EXTERNAL_VARIABLE; // Variable
    }
    
    // Verificar si es una variable ya declarada
    if (es_variable(token)) return EXTERNAL_VARIABLE;
    
    // Por defecto se interpreta como una cadena de texto
    return CONSTANT_STRING;
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
    if (line[pos] == '\0' || line[pos] == '\n') return pos; // Si se detecta un fin de cadena o salto de línea se devuelve directamente la posición

    // Manejo para el fin de instrucción :_: de infinix
    if (line[pos] == ':' && line[pos+1] == '_' && line[pos+2] == ':') {
        strcpy(token, ":_:");
        return pos + 3; // Devuelve el valor actual de la posición de lectura más 3 posiciones correspondientes a los tres elementos del fin de instrucción
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

    // Manejo de cadenas con la palabra reservada @ de infinix
    if (line[pos] == '@') { // Inicio de la cadena
        token[i++] = line[pos++]; // Se pasa al siguiente carácter del token
        // Si se detecta un espacio, tabulación, fin de cadena, fin de línea, el operador menor o mayor
        if (line[pos] == ' ' || line[pos] == '\t' || line[pos] == '\0' || line[pos] == '\n') { 
            token[i] = '\0'; // Fin de lectura de cadena
            return pos; // Devolver la posición actual
        }
        // Válidar cadenas que se encuentren entre @ @
        while (line[pos] != '@' && line[pos] != '\0' && line[pos] != '\n') {
            token[i++] = line[pos++]; // Se va iterando y construyendo el valor del token 
        }
        if (line[pos] == '@') { // Cierre de cadena, se incluye el @
            token[i++] = line[pos++];
        }
        token[i] = '\0';
        return pos; // Se devuelve la posició actual
    }

    // Manejo de operadores de un solo carácter (se removió ' de aquí ya que se maneja arriba)
    if (strchr("=<>+-*/?@:,", line[pos]) != NULL) {
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
 * Función para verificar si es identificador válido
 */
bool es_identificador_valido(const char *str) {
    if (strlen(str) == 0) return false;
    
    // Debe empezar con letra o guión bajo
    if (!isalpha(str[0]) && str[0] != '_') return false;
    
    // El resto pueden ser letras, dígitos o guión bajo
    for (int i = 1; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && str[i] != '_') return false;
    }
    return true;
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
 * Función para agregar un token a la tabla de tokens
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
 * Función para escribir el contenido del token al archivo objeto según su clasificación (tipo) de símbolo
 * (Versión mejorada con soporte para constantes de carácter y booleanas)
 * @param token: Para el token a escribir
 * @param type: Para el tipo de token
 */
void escribir_contenido_obj(const char *token, SymbolType type) {
    switch(type) {
        case INTERNAL_SYMBOL:
        // Para palabras reservadas y operadores se escriben (se dejan pasar) tal como son
            fprintf(output_file, "%d %s\n", current_line, token); // Se escribe la línea y el valor del token
            break;
        case EXTERNAL_VARIABLE:
            fprintf(output_file, "%d var\n", current_line); // Se escribe la línea y el valor del token
            break;
        case CONSTANT_STRING:
        // Validar contenido entre los @ correspondiente a una cadena
            if (token[0] == '@' && token[strlen(token) - 1] == '@' && strlen(token) > 2) {
                fprintf(output_file, "%d @\n", current_line);
                fprintf(output_file, "%d const_cadena\n", current_line);
                fprintf(output_file, "%d @\n", current_line);
            } else if (strcmp(token, "@") == 0) {
                fprintf(output_file, "%d @\n", current_line);
            }
            else{
                fprintf(output_file, "%d const_cadena\n", current_line);
            }
            break;
        case CONSTANT_CHAR:
            // Para constantes de carácter se escribe const_caracter
            if(token[0] == '\'' && token[strlen(token) - 1] == '\'' && strlen(token) == 3 ){
                fprintf(output_file, "%d '\n", current_line);
                fprintf(output_file, "%d const_caracter\n", current_line); // Se escribe la línea y la clasificación
                fprintf(output_file, "%d '\n", current_line);
            }
            break;
        case CONSTANT_NUMBER:
            fprintf(output_file, "%d const_numerica\n", current_line); // Se escribe la línea y el valor del token
            break;
        case CONSTANT_BOOLEAN:
            // Para constantes booleanas se escribe const_booleana
            fprintf(output_file, "%d const_booleana\n", current_line); // Se escribe la línea y la clasificación
            break;
    }
}

// Variables globales para mantener el estado entre líneas
static bool es_declaracion_global = false; // Bandera global para declaración de variables
static char ultimo_token[MAX_LINE_LENGTH] = ""; // Para almacenar el último token procesado

// ==================== DECLARACIONES FORWARD ====================
// Agregar estas declaraciones después de las variables globales y antes de las funciones

// Funciones del análisis léxico
void inicializar_tabla_interna_simbolos();
bool es_palabra_reservada(const char *name);
bool es_constante_booleana(const char *token);
bool es_constante_caracter(const char *token);
bool es_variable(const char *name);
void agregar_variable(const char *name);
bool es_numero(const char *token);
SymbolType clasificacion_tokens(const char *token, bool es_declaracion);
int obtener_siguiente_token(const char *line, int pos, char *token);
bool es_identificador_valido(const char *str);
TokenType clasificar_token(const char *lexema);
void agregar_token(const char *lexema, int linea);
void escribir_contenido_obj(const char *token, SymbolType type);
bool es_linea_vacia(const char *line);
void procesar_linea_codigo(const char *line);
void analisis_lexico(FILE *source_file);
void imprimir_tabla_tokens();

// Funciones del análisis sintáctico
Token* get_current_token();
void next_token();
bool match(TokenType expected_type);
void syntax_error_msg(const char* expected);
void panic_mode_sync();
bool parse_programa();
bool parse_instruccion();
bool parse_declaracion();
bool parse_tipo_dato();
bool parse_lista_declaraciones();
bool parse_declaracion_simple();
bool parse_asignacion();
bool parse_instruccion_salida();
bool parse_instruccion_entrada();
bool parse_lista_elementos_salida();
bool parse_elemento_salida();
bool parse_expresion();
bool parse_termino();
bool parse_factor();
bool parse_cadena_literal();
bool analisis_sintactico();

// Funciones de utilidad
const char* get_token_name(TokenType type);
void imprimir_tabla_simbolos();

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
                strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
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

void analisis_lexico(FILE *source_file);

/**
 * Función para realizar el análisis léxico completo
 * @param source_file: Es el código fuente a analizar
 */
void analisis_lexico(FILE *source_file) {
    char line[MAX_LINE_LENGTH];
    int line_number = 1;
    
    while (fgets(line, sizeof(line), source_file)) {
        char token[MAX_LINE_LENGTH];
        int pos = 0;
        int line_len = strlen(line);
        
        while (pos < line_len) {
            // Saltar espacios en blanco
            while (pos < line_len && (line[pos] == ' ' || line[pos] == '\t')) pos++;
            if (pos >= line_len || line[pos] == '\n') break;
            
            int i = 0;
            
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
                if (pos < line_len && line[pos] != '\'') {
                    token[i++] = line[pos++];
                }
                if (pos < line_len && line[pos] == '\'') {
                    token[i++] = line[pos++];
                }
                token[i] = '\0';
                agregar_token(token, line_number);
                continue;
            }
            
            // Manejo de cadenas @texto@
            if (line[pos] == '@') {
                token[i++] = line[pos++];
                while (pos < line_len && line[pos] != '@' && line[pos] != '\n') {
                    token[i++] = line[pos++];
                }
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
                   line[pos] != '\n' && strchr("=<>+-*/,:@'", line[pos]) == NULL) {
                token[i++] = line[pos++];
            }
            
            if (i > 0) {
                token[i] = '\0';
                agregar_token(token, line_number);
            }
        }
        line_number++;
    }
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

// ==================== ANÁLISIS SINTÁCTICO ====================

/**
 * Función para obtener el token actual
 */
Token* get_current_token() {
    if (current_token >= token_count) {
        static Token eof_token = {TOKEN_EOF, "EOF", -1};
        return &eof_token;
    }
    return &tokens[current_token];
}

/**
 * Función para avanzar al siguiente token
 */
void next_token() {
    if (current_token < token_count) {
        current_token++;
    }
}

/**
 * Función para verificar si el token actual es del tipo esperado
 */
bool match(TokenType expected_type) {
    Token* token = get_current_token();
    if (token->type == expected_type) {
        next_token();
        return true;
    }
    return false;
}

/**
 * Función para reportar errores sintácticos
 */
void syntax_error_msg(const char* expected) {
    syntax_error = true;
    Token* token = get_current_token();
    
    // Imprimir inmediatamente en lugar de solo guardar
    printf("ERROR SINTÁCTICO en línea %d: se esperaba %s, se encontró '%s'\n", 
           token->linea, expected, token->lexema);
    
    // También guardar para referencia
    snprintf(error_message, sizeof(error_message), 
             "Error sintáctico en línea %d: se esperaba %s, se encontró %s", 
             token->linea, expected, token->lexema);
}

/**
 * Función para entrar en modo pánico y sincronizar
 * Descarta tokens hasta encontrar un punto de sincronización seguro
 */
void panic_mode_sync() {
    // printf("Entrando en modo pánico en línea %d...\n", get_current_token()->linea);
    
    // Descartar tokens hasta encontrar un punto de sincronización
    while (get_current_token()->type != TOKEN_EOF) {
        TokenType current = get_current_token()->type;
        
        // Verificar si es un token de sincronización
        for (int i = 0; i < sizeof(sync_tokens)/sizeof(sync_tokens[0]); i++) {
            if (current == sync_tokens[i]) {
                /*printf("Sincronizando en token: %s (línea %d)\n", 
                       get_current_token()->lexema, get_current_token()->linea);*/
                return; // Salir del modo pánico
            }
        }
        
        // Si es :_: (fin de instrucción), consumirlo y salir
        if (current == TOKEN_FINLINEA) {
            next_token(); // Consumir el :_:
            printf("Sincronizando después de :_: (línea %d)\n", get_current_token()->linea);
            return;
        }
        
        next_token(); // Descartar token actual
    }
}

/**
 * <programa> ::= <instruccion>*
 */
bool parse_programa() {
    bool has_errors = false;
    
    while (get_current_token()->type != TOKEN_EOF) {
        if (!parse_instruccion()) {
            has_errors = true;
            // NO retornar false aquí, continuar con la siguiente instrucción
            
            // Si ya estamos en EOF después del pánico, salir
            if (get_current_token()->type == TOKEN_EOF) {
                break;
            }
        }
    }
    
    return !has_errors; // Retornar true solo si no hubo errores
}

/**
 * <instruccion> ::= <declaracion> ":_:" | <asignacion> ":_:" | <instruccion_salida> ":_:" | <instruccion_entrada> ":_:"
 */
/**
 * Versión mejorada con modo pánico
 */
bool parse_instruccion() {
    Token* token = get_current_token();
    
    // Verificar qué tipo de instrucción es
    if (token->type == TOKEN_ENTERO || token->type == TOKEN_DECIMAL || 
        token->type == TOKEN_CARACTER || token->type == TOKEN_CADENA || 
        token->type == TOKEN_BOOLEANO) {
        // Es una declaración
        if (!parse_declaracion()) {
            panic_mode_sync(); // Entrar en modo pánico
            return false; // Pero continuar con el análisis
        }
    } else if (token->type == TOKEN_SALIDA) {
        if (!parse_instruccion_salida()) {
            panic_mode_sync();
            return false;
        }
    } else if (token->type == TOKEN_ENTRADA) {
        if (!parse_instruccion_entrada()) {
            panic_mode_sync();
            return false;
        }
    } else if (token->type == TOKEN_IDENTIFICADOR) {
        if (!parse_asignacion()) {
            panic_mode_sync();
            return false;
        }
    } else {
        syntax_error_msg("declaración, asignación, instrucción de entrada o salida");
        panic_mode_sync(); // Sincronizar y continuar
        return false;
    }
    
    // Verificar fin de instrucción
    if (!match(TOKEN_FINLINEA)) {
        syntax_error_msg(":_:");
        panic_mode_sync();
        return false;
    }
    
    return true;
}

// ==================== FUNCIONES FALTANTES ====================
// Agregar estas funciones antes de la función main()

/**
 * <declaracion> ::= <tipo_dato> <lista_declaraciones>
 */
bool parse_declaracion() {
    if (!parse_tipo_dato()) return false;
    if (!parse_lista_declaraciones()) return false;
    return true;
}

/**
 * <tipo_dato> ::= "entero" | "decimal" | "caracter" | "cadena" | "booleano"
 */
bool parse_tipo_dato() {
    Token* token = get_current_token();
    
    if (token->type == TOKEN_ENTERO || token->type == TOKEN_DECIMAL ||
        token->type == TOKEN_CARACTER || token->type == TOKEN_CADENA ||
        token->type == TOKEN_BOOLEANO) {
        next_token();
        return true;
    }
    
    syntax_error_msg("tipo de dato");
    return false;
}

/**
 * <lista_declaraciones> ::= <declaracion_simple> ("," <declaracion_simple>)*
 */
bool parse_lista_declaraciones() {
    if (!parse_declaracion_simple()) return false;
    
    while (match(TOKEN_COMA)) {
        if (!parse_declaracion_simple()) return false;
    }
    
    return true;
}

/**
 * <declaracion_simple> ::= IDENTIFICADOR ("=" <expresion>)?
 */
bool parse_declaracion_simple() {
    if (!match(TOKEN_IDENTIFICADOR)) {
        syntax_error_msg("identificador");
        return false;
    }
    
    // Asignación opcional
    if (match(TOKEN_IGUAL)) {
        if (!parse_expresion()) return false;
    }
    
    return true;
}

/**
 * <asignacion> ::= IDENTIFICADOR "=" <expresion>
 */
bool parse_asignacion() {
    if (!match(TOKEN_IDENTIFICADOR)) {
        syntax_error_msg("identificador");
        return false;
    }
    
    if (!match(TOKEN_IGUAL)) {
        syntax_error_msg("=");
        return false;
    }
    
    if (!parse_expresion()) return false;
    
    return true;
}

/**
 * <instruccion_salida> ::= "mt" "<" <lista_elementos_salida> ">"
 */
bool parse_instruccion_salida() {
    if (!match(TOKEN_SALIDA)) {
        syntax_error_msg("mt");
        return false;
    }
    
    if (!match(TOKEN_MENOR)) {
        syntax_error_msg("<");
        return false;
    }
    
    if (!parse_lista_elementos_salida()) return false;
    
    if (!match(TOKEN_MAYOR)) {
        syntax_error_msg(">");
        return false;
    }
    
    return true;
}

/**
 * <instruccion_entrada> ::= "inp" "<" IDENTIFICADOR ">"
 */
bool parse_instruccion_entrada() {
    if (!match(TOKEN_ENTRADA)) {
        syntax_error_msg("inp");
        return false;
    }
    
    if (!match(TOKEN_MENOR)) {
        syntax_error_msg("<");
        return false;
    }
    
    if (!match(TOKEN_IDENTIFICADOR)) {
        syntax_error_msg("identificador");
        return false;
    }
    
    if (!match(TOKEN_MAYOR)) {
        syntax_error_msg(">");
        return false;
    }
    
    return true;
}

/**
 * <lista_elementos_salida> ::= <elemento_salida> ("," <elemento_salida>)*
 */
bool parse_lista_elementos_salida() {
    if (!parse_elemento_salida()) return false;
    
    while (match(TOKEN_COMA)) {
        if (!parse_elemento_salida()) return false;
    }
    
    return true;
}

/**
 * <elemento_salida> ::= <cadena_literal> | <expresion>
 */
bool parse_elemento_salida() {
    Token* token = get_current_token();
    
    if (token->type == TOKEN_CONSTANTE_CADENA) {
        return parse_cadena_literal();
    } else {
        return parse_expresion();
    }
}

/**
 * <expresion> ::= <termino> (("+" | "-") <termino>)*
 */
bool parse_expresion() {
    if (!parse_termino()) return false;
    
    while (get_current_token()->type == TOKEN_MAS || get_current_token()->type == TOKEN_MENOS) {
        next_token();
        if (!parse_termino()) return false;
    }
    
    return true;
}

/**
 * <termino> ::= <factor> (("*" | "/") <factor>)*
 */
bool parse_termino() {
    if (!parse_factor()) return false;
    
    while (get_current_token()->type == TOKEN_POR || get_current_token()->type == TOKEN_DIV) {
        next_token();
        if (!parse_factor()) return false;
    }
    
    return true;
}

/**
 * <factor> ::= NUMERO | IDENTIFICADOR | "<" <expresion> ">" | "'" CARACTER "'" | <cadena_literal>
 */
bool parse_factor() {
    Token* token = get_current_token();
    
    if (match(TOKEN_NUMERO)) {
        return true;
    } else if (match(TOKEN_IDENTIFICADOR)) {
        return true;
    } else if (match(TOKEN_MENOR)) {
        if (!parse_expresion()) return false;
        if (!match(TOKEN_MAYOR)) {
            syntax_error_msg(">");
            return false;
        }
        return true;
    } else if (match(TOKEN_CONSTANTE_CHAR)) {
        return true;
    } else if (token->type == TOKEN_CONSTANTE_CADENA) {
        return parse_cadena_literal();
    } else if (match(TOKEN_CONSTANTE_BOOL)) {
        return true;
    } else {
        syntax_error_msg("número, identificador, carácter o cadena");
        return false;
    }
}

/**
 * <cadena_literal> ::= "@" (CARACTER)* "@"
 */
bool parse_cadena_literal() {
    if (!match(TOKEN_CONSTANTE_CADENA)) {
        syntax_error_msg("cadena literal");
        return false;
    }
    return true;
}

/**
 * Función principal del análisis sintáctico
 */
bool analisis_sintactico() {
    current_token = 0;
    syntax_error = false;
    
    printf("\n=== ANÁLISIS SINTÁCTICO ===\n");
    
    bool result = parse_programa();
    
    if (syntax_error) {
        printf("ERROR: %s\n", error_message);
        return false;
    } else if (result) {
        printf("✓ Análisis sintáctico completado exitosamente\n");
        printf("✓ El programa es sintácticamente correcto\n");
        return true;
    } else {
        printf("ERROR: Análisis sintáctico falló\n");
        return false;
    }
}

// Función para imprimir (mostrar) el contenido de las tablas de símbolos en la consola
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

/**
 * Función principal - Secuencia corregida
 */
int main() {
    printf("=== COMPILADOR INFINIX ===\n\n");
    
    // 1. INICIALIZACIÓN
    printf("Inicializando tabla de símbolos interna...\n");
    inicializar_tabla_interna_simbolos();
    
    // 2. APERTURA DE ARCHIVOS    
    FILE *source_file = fopen("codigo_fuente.txt", "r");
    if (source_file == NULL) {
        printf("Error: No se pudo abrir el archivo de entrada 'codigo_fuente.txt'\n");
        return 1;
    }

    output_file = fopen("codigo_objeto.obj", "w");
    if (output_file == NULL) {
        printf("Error: No se pudo crear el archivo de salida 'codigo_objeto.obj'\n");
        fclose(source_file);
        return 1;
    }
    
    // 3. ANÁLISIS LÉXICO (PRIMERA PASADA)
    printf("Iniciando análisis léxico...\n");
    analisis_lexico(source_file);
    
    // 4. MOSTRAR RESULTADOS DEL ANÁLISIS LÉXICO
    imprimir_tabla_tokens();
    printf("\nTokens analizados: %d\n", token_count);
    printf("Análisis léxico: ✓ Completado\n");
    
    // Reinicio de lectura del archivo codigo_fuente.txt
    rewind(source_file);  // Volver al inicio del archivo
    
    // 6. PROCESAMIENTO LÍNEA POR LÍNEA (SEGUNDA PASADA)    
    char line[MAX_LINE_LENGTH];
    current_line = 1;

    while (fgets(line, sizeof(line), source_file)) {
        if (strlen(line) > 0) {
            procesar_linea_codigo(line);
        }
        current_line++;
    }
    
    // 7. ANÁLISIS SINTÁCTICO
    printf("\n6. Iniciando análisis sintáctico...\n");
    bool sintaxis_correcta = analisis_sintactico();
    printf("Análisis sintáctico: %s\n\n", sintaxis_correcta ? "✓ Exitoso" : "✗ Con errores");
    
    // 8. MOSTRAR TABLAS DE SÍMBOLOS
    printf("Mostrando tablas de símbolos...\n");
    imprimir_tabla_simbolos();
    
    // 9. CIERRE DE ARCHIVOS
    fclose(source_file);
    fclose(output_file);
    
    // 10. MENSAJE FINAL
    printf("\n=== PROCESAMIENTO COMPLETADO ===\n");
    printf("- Archivo objeto generado: codigo_objeto.obj\n");
    printf("- Estado: %s\n", sintaxis_correcta ? "✓ EXITOSO" : "✗ CON ERRORES");
    
    return sintaxis_correcta ? 0 : 1;
}