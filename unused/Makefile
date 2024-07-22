CXX         = clang++
CXXFLAGS    = -c -g -std=c++20
WARNINGS    = -pedantic-errors -Wall -Wextra -Weffc++ -fsanitize=undefined -Wno-unused-parameter
LDFLAGS     = -fsanitize=undefined
LDLIBS      = -lncurses
DEPS        = $(wildcard *.hpp)
EXECUTABLE  = bin
DEFINES     =

BuildDir=build

TranslationUnits:= $(shell find -type f -name "*.cpp") #$(wildcard *.cpp)
Objects:= $(addprefix $(BuildDir)/,$(TranslationUnits:.cpp=.o))

.PHONY: all
all: $(EXECUTABLE)
	@echo 'all' done.

$(EXECUTABLE): $(Objects)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BuildDir)/%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(DEFINES) $< -o $@


.PHONY: clean run
clean:
	rm -f $(Objects) $(EXECUTABLE)

run: $(EXECUTABLE)
	./bin
