#include "tipos.h"
#include "tabla_simbolos.h"
#include "lexico.h"
#include "sintactico.h"

int main() {
    printf("=== COMPILADOR INFINIX MODULARIZADO ===\n\n");
    
    // 1. INICIALIZACIÓN
    printf("Inicializando tabla de símbolos interna...\n");
    inicializar_tabla_interna_simbolos();
    
    // 2. APERTURA DE ARCHIVOS    
    FILE *source_file = fopen("codigo_fuente.txt", "r");
    if (source_file == NULL) {
        printf("Error: No se pudo abrir el archivo de entrada 'codigo_fuente.txt'\n");
        return 1;
    }

    FILE *output_file = fopen("codigo_objeto.obj", "w");
    if (output_file == NULL) {
        printf("Error: No se pudo crear el archivo de salida 'codigo_objeto.obj'\n");
        fclose(source_file);
        return 1;
    }
    
    // Configurar archivo de salida para el analizador léxico
    set_output_file(output_file);
    
    // 3. ANÁLISIS LÉXICO Y GENERACIÓN DE CÓDIGO OBJETO
    printf("Iniciando análisis léxico y generación de código objeto...\n");
    
    // Reiniciar el archivo para procesamiento línea por línea
    rewind(source_file);
    
    char line[MAX_LINE_LENGTH];
    current_line = 1;
    
    // Procesar cada línea del código fuente
    while (fgets(line, sizeof(line), source_file)) {
        // Procesar la línea para generar código objeto
        procesar_linea_codigo(line);
        current_line++;
    }
    
    // También hacer el análisis léxico para la tabla de tokens (opcional, para debugging)
    rewind(source_file);
    current_line = 1;
    analisis_lexico(source_file);
    
    // Mostrar tabla de tokens (opcional)
    imprimir_tabla_tokens();
    
    // 4. ANÁLISIS SINTÁCTICO
    printf("\nIniciando análisis sintáctico...\n");
    bool sintaxis_correcta = analisis_sintactico();
    
    // 5. MOSTRAR RESULTADOS
    imprimir_tabla_simbolos();
    
    // 6. CIERRE
    fclose(source_file);
    fclose(output_file);
    
    printf("\n=== PROCESAMIENTO COMPLETADO ===\n");
    printf("Estado: %s\n", sintaxis_correcta ? "✓ EXITOSO" : "✗ CON ERRORES");
    printf("Código objeto generado en: codigo_objeto.obj\n");
    
    return sintaxis_correcta ? 0 : 1;
}