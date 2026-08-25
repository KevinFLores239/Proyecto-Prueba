# Directorios de origen y destino
SRC_DIR := src
BIN_DIR := bin

# Detectar sistema operativo
ifeq ($(OS),Windows_NT)
	# Configuración para Windows
	CXX := g++
	CXXFLAGS := -std=c++17 -Wall -Iinclude -IC:/SFML/include
	LDFLAGS := -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	EXE_EXT := .exe
else
	# Configuración para macOS/Linux
	CXX := g++
	CXXFLAGS := -std=c++17 -Wall -Iinclude -I/opt/homebrew/opt/sfml@2/include
	LDFLAGS := -L/opt/homebrew/opt/sfml@2/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	EXE_EXT := .exe
endif

# Obtener todos los archivos .cpp en el directorio de origen
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)

# Generar los nombres de los archivos ejecutables en el directorio de destino
EXE_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%$(EXE_EXT),$(CPP_FILES))

# Regla para compilar cada archivo .cpp y generar el archivo ejecutable correspondiente
$(BIN_DIR)/%$(EXE_EXT): $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Crear el directorio bin si no existe
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Regla por defecto para compilar todos los archivos .cpp
all: $(EXE_FILES)

# Regla para ejecutar cada archivo ejecutable
run%: $(BIN_DIR)/%$(EXE_EXT)
	./$<

# Regla para limpiar los archivos generados
clean:
	rm -f $(EXE_FILES)

.PHONY: all clean run%
