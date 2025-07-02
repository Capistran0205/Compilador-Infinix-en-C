#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "tipos.h"
#include "lexico.h"
// Funciones del anális semántico
// Verificar si variable está declarada
bool verificar_variable_declarada(const char* nombre);

// Verificar compatibilidad de tipos
bool verificar_compatibilidad_tipos(DataType tipo1, DataType tipo2);

// Verificar asignación
bool verificar_asignacion(const char* variable, DataType tipo_valor);

// Verificar operación aritmética
bool verificar_operacion_aritmetica(DataType tipo1, DataType tipo2, char operador);

// Verificar llamada a función (mt, inp)
bool verificar_instruccion_entrada_salida(TokenType instruccion, DataType* tipos_parametros);

#endif // SEMANTICO_H