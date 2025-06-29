#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Valores constantes para la cantidad total de símbolos, identificadores y la longitud de la línea 
#define MAX_SYMBOLS 100 // Total de palabras reservadas
#define MAX_IDENTIFIERS 100 // Total de variables 
#define MAX_LINE_LENGTH 256 // Máximos de carácteres por línea

/**
 * Enumerable SymbolType para representar los cuatro elementos esenciales del lenguaje Infinix
 * INTERNAL_SYMBOL: Para palabras reservadas para tipos de datos y funciones
 * EXTERNAL_VARIABLE: Para a declaración de variables
 * CONSTANT_STRING: Para declaración de constantes de tipo cadena
 * CONSTANT_CHAR: Para declaración de constantes de tipo carácter
 * CONSTANT_NUMBER: Para declaración de constantes de tipo númerica
 * CONSTANT_BOOLEAN: Para declaración de constantes de tipo booleano
 */
typedef enum {
    INTERNAL_SYMBOL,
    EXTERNAL_VARIABLE,
    CONSTANT_STRING,
    CONSTANT_CHAR,
    CONSTANT_NUMBER,
    CONSTANT_BOOLEAN
} SymbolType;


/**
 * Struct para representar la entrada de símbolos a la tabla
 * name: Representa la descripción del símbolo
 * SymbolType: Representa el tipo de símbolo
 */
typedef struct {
    char name[50];
    SymbolType type;
} SymbolEntry;


// Tabla de símbolos globales
SymbolEntry internal_symbols[MAX_SYMBOLS]; // Tabla para palabras reservadas, operadores)
SymbolEntry external_symbols[MAX_IDENTIFIERS]; // Tabla externa (variables declaradas)

// Contadores de símbolos internos y externos pertencientes a las tablas de símbolos previamente definidas
int internal_count = 0; // Representa el contatodr de palabras reservadas detectadas.
int external_count = 0; // Representa el contador de variables detectadas
int current_line = 1; // Para la Línea actual de código fuente
FILE *output_file; // Es el Archivo de salida para el código objeto .obj


/**
 * Función para inicializar la tabla interna de símbolos para las palabras reservadas y operadores 
 * del lenguaje infinix. No recibe parámetros ni retorna un valor
 */
void inicializar_tabla_interna_simbolos() {
    // Palabras reservadas (incluye true y false para constantes booleanas)
    char *keywords[] = {"entero", "decimal", "caracter", "cadena", "booleano", "inp", "mt", "true", "false"};
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, keywords[i]); // Se agrega la descripción del símbolo al arreglo de la tabla interna
    }

    // Operadores arítmeticos y símbolos del lenguaje infinix
    char *symbols[] = {"<", ">", ":_:", "=", "+", "-", "*", "/", "@", ",", "'"};
    for (int i = 0; i < sizeof(symbols)/sizeof(symbols[0]); i++) {
        strcpy(internal_symbols[internal_count++].name, symbols[i]); // Se agrega la descripción del símbolo al arreglo de la tabla interna
    }

    // Se recorre los símbolos agregados a la tabla interna y se clasifican como INTERNAL_SYMBOL
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
            } else {
                fprintf(output_file, "%d const_cadena\n", current_line); // Se escribe la línea y el valor del token
            }
            break;
        case CONSTANT_CHAR:
            // Para constantes de carácter se escribe const_caracter
            fprintf(output_file, "%d const_caracter\n", current_line); // Se escribe la línea y la clasificación
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

/**
 * Función para procesar una línea completa de código fuente (puntero char), no devuelve valor
 * Versión mejorada que evita la duplicación de tokens en líneas multilínea
 * @param line: La línea de código a procesar
 */
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
 * Función principal de ejecución
 */
int main() {
    // Inicialización de la tabla de símbolos interna (palabras reservadas y operadores)
    inicializar_tabla_interna_simbolos();

    // Abre el archivo de código fuente
    FILE *source_file = fopen("codigo_fuente.txt", "r");
    if (source_file == NULL) {
        printf("Error: No se pudo abrir el archivo de entrada\n");
        return 1;
    }

    // Se crea el archivo de código objeto
    output_file = fopen("codigo_objeto.obj", "w");
    if (output_file == NULL) {
        printf("Error: No se pudo crear el archivo de salida\n");
        fclose(source_file);
        return 1;
    }
    
    char line[MAX_LINE_LENGTH];
    current_line = 1;

    // Procesamiento de cada línea del archivo fuente
    while (fgets(line, sizeof(line), source_file)) {
        if (strlen(line) > 0) {
            procesar_linea_codigo(line); // Procesamiento de línea
        }
        current_line++; // Incremento de línea actual (siempre se incrementa para mantener numeración correcta)
    }

    // Impresión de las tablas de símbolos en la consola
    imprimir_tabla_simbolos();

    // Cierre de archivos fuente y código objeto respectivamente
    fclose(source_file);
    fclose(output_file);

    printf("Procesamiento completado correctamente.\n");
    return 0;
}