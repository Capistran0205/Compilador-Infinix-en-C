#include "sintactico.h"

// Definición de variables globales
int current_token = 0;
bool syntax_error = false;
char error_message[256];

// Implementar todas las funciones del análisis sintáctico aquí...
// (Copiar las funciones correspondientes del archivo original)

/**
 * Función principal del análisis sintáctico
 */
bool analisis_sintactico() {
    current_token = 0;
    syntax_error = false;
    
    printf("\n=== ANÁLISIS SINTÁCTICO ===\n");
    
    bool result = parse_programa();
    
    if (syntax_error) {
        // printf("ERROR: %s\n", error_message);
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

/**
 * <programa> ::= <instruccion>*
 */
bool parse_programa() {
    bool has_errors = false;
    
    while (get_current_token()->type != TOKEN_EOF) {
        // Verificar si estamos en un estado válido para iniciar instrucción
        TokenType current = get_current_token()->type;
        
        if (current != TOKEN_ENTERO && current != TOKEN_DECIMAL &&
            current != TOKEN_CARACTER && current != TOKEN_CADENA &&
            current != TOKEN_BOOLEANO && current != TOKEN_SALIDA &&
            current != TOKEN_ENTRADA && current != TOKEN_IDENTIFICADOR) {
            
            // Token inesperado al inicio de instrucción
            syntax_error_msg("inicio de instrucción válido(declaración de variable, entrada o salida de datos)");
            panic_mode_sync();
            has_errors = true;
            continue;
        }
        
        // Intentar parsear la instrucción
        if (!parse_instruccion()) {
            has_errors = true;
            // El modo pánico ya se ejecutó en parse_instruccion()
            // Solo continuar si no estamos en EOF (Fin del Archivo)
            if (get_current_token()->type == TOKEN_EOF) {
                break;
            }
        }
    }
    
    return !has_errors;
}

/**
 * <declaracion> ::= <tipo_dato> <lista_declaraciones>
 */
bool parse_declaracion() {
    if (!parse_tipo_dato()) return false;
    if (!parse_lista_declaraciones()) return false;
    return true;
}

/**
 * <instruccion> ::= <declaracion> ":_:" | <asignacion> ":_:" | <instruccion_salida> ":_:" | <instruccion_entrada> ":_:"
 */
/**
 * Versión mejorada con modo pánico
 */
bool parse_instruccion() {
    Token* token = get_current_token();
    bool instruction_parsed = false;
    
    // Intentar parsear según el tipo de instrucción
    if (token->type == TOKEN_ENTERO || token->type == TOKEN_DECIMAL || 
        token->type == TOKEN_CARACTER || token->type == TOKEN_CADENA || 
        token->type == TOKEN_BOOLEANO) {
        instruction_parsed = parse_declaracion();
    } else if (token->type == TOKEN_SALIDA) {
        instruction_parsed = parse_instruccion_salida();
    } else if (token->type == TOKEN_ENTRADA) {
        instruction_parsed = parse_instruccion_entrada();
    } else if (token->type == TOKEN_IDENTIFICADOR) {
        instruction_parsed = parse_asignacion();
    } else {
        syntax_error_msg("declaración, asignación, instrucción de entrada o salida");
        panic_mode_sync();
        return false;
    }
    
    // Si falló el parsing de la instrucción
    if (!instruction_parsed) {
        panic_mode_sync();
        return false;
    }
    
    // Verificar fin de instrucción (:_:)
    if (get_current_token()->type != TOKEN_FINLINEA) {
        syntax_error_msg(":_:");
        panic_mode_sync();
        return false;
    }
    
    // Consumir el :_:
    next_token();
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
    /*snprintf(error_message, sizeof(error_message), 
             "Error sintáctico en línea %d: se esperaba %s, se encontró %s", 
             token->linea, expected, token->lexema);*/
}

/**
 * Función para entrar en modo pánico y sincronizar
 * Descarta tokens hasta encontrar un punto de sincronización seguro
 */
void panic_mode_sync() {
    // printf("Iniciando recuperación de error...\n");
    
    // Consumir tokens hasta encontrar sincronización segura
    while (get_current_token()->type != TOKEN_EOF) {
        TokenType current = get_current_token()->type;
        
        // Si encontramos :_: (fin de instrucción), lo consumimos y terminamos
        if (current == TOKEN_FINLINEA) {
            next_token(); // CONSUMIR el :_:
            // printf("Recuperación: Sincronizado después de :_:\n");
            return;
        }
        
        // Si encontramos inicio de nueva instrucción, NO lo consumimos
        if (current == TOKEN_ENTERO || current == TOKEN_DECIMAL ||
            current == TOKEN_CARACTER || current == TOKEN_CADENA ||
            current == TOKEN_BOOLEANO || current == TOKEN_SALIDA ||
            current == TOKEN_ENTRADA) {
            // printf("Recuperación: Sincronizado en inicio de instrucción\n");
            return; // NO consumir, dejar para el próximo parse
        }
        
        // Consumir token actual y continuar
        // printf("Descartando token: %s\n", get_current_token()->lexema);
        next_token();
    }
    
    printf("Recuperación: Alcanzado EOF\n");
}

/**
 * Función que se encarga de validar si una instrucción es correcta en el comienzo para el lenguaje
 * @return true si hay alguna coincidencia y en caso contrario false
 */
bool is_valid_instruction_start() {
    TokenType current = get_current_token()->type;
    return (current == TOKEN_ENTERO || current == TOKEN_DECIMAL ||
            current == TOKEN_CARACTER || current == TOKEN_CADENA ||
            current == TOKEN_BOOLEANO || current == TOKEN_SALIDA ||
            current == TOKEN_ENTRADA || current == TOKEN_IDENTIFICADOR);
}