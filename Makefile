# Variables del compilador
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinc -Isrc/buffers -lm -lSDL2 -lSDL2_ttf -lSDL2_image -lunistring -g
#CXXFLAGS = -Wall -std=c++17 -Iinc -lunistring -g `MagickWand-config --cflags --cppflags` `MagickWand-config --ldflags --libs`
# CXXFLAGS = -Wall -std=c++17 -Iinc -Isrc/buffers (cada vez que quiera meter una carpeta más dentro del src, linkear así) -lSDL2 -lSDL2_ttf -lSDL2_image -lunistring -g
# TODO: Implementar que pueda generar imágenes también.

# Directorios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Nombre del ejecutable
TARGET = $(BIN_DIR)/mathparser

# Encontrar todos los archivos .cpp dentro de src y sus subdirectorios
SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
# Los archivos .o se generarán en la carpeta obj
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Reglas
all: $(BIN_DIR) $(OBJ_DIR) $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compilar el ejecutable final
$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_FILES) -ly -lfl

# Regla genérica para compilar archivos .cpp
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean

