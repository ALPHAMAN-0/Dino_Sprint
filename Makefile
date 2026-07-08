CXX      := clang++
# Sources are grouped into src/ subfolders. Add each to the header search
# path so the bare `#include "Foo.h"` lines resolve from any folder.
INC      := -Isrc -Isrc/core -Isrc/characters -Isrc/background -Isrc/entities -Isrc/ui
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -DGL_SILENCE_DEPRECATION -MMD -MP $(INC)
LDLIBS   := -framework GLUT -framework OpenGL
SRC      := $(wildcard src/*.cpp src/*/*.cpp)
OBJ      := $(SRC:.cpp=.o)
DEP      := $(OBJ:.o=.d)
BIN      := dino_sprint

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDLIBS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(BIN)

clean:
	rm -f $(OBJ) $(DEP) $(BIN)
	find src \( -name '*.o' -o -name '*.d' \) -delete

-include $(DEP)

.PHONY: all run clean
