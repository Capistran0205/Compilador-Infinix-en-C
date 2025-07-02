# Para ejecutar primero escribir en terminal: make clean && make creando los archivos .o (compilando y creando los ejecutables de cada .c)
# Luego con make run se unifica la ejecución de todos los .o
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Archivos fuente
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = compilador_infinix

# Regla principal
all: $(TARGET)

# Crear directorio de objetos si no existe
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compilar ejecutable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

# Compilar archivos objeto
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Ejecutar el compilador
run: $(TARGET)
	./$(TARGET)

# Depuración
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean run debug