CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -DGL_SILENCE_DEPRECATION -Ivendor -MMD -MP
LDLIBS   := -framework GLUT -framework OpenGL
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:.cpp=.o)
DEP      := $(OBJ:.o=.d)
BIN      := dino_sprint

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDLIBS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run from the repo root so the relative assets/ path resolves.
run: all
	./$(BIN)

clean:
	rm -f $(OBJ) $(DEP) $(BIN)

-include $(DEP)

.PHONY: all run clean
