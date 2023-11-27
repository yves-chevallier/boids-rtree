EXEC=myapp

all: $(EXEC)

gprof: analysis.txt

$(EXEC): main.cpp
	g++ -Os -pg $< -o $(EXEC) -lsfml-graphics -lsfml-window -lsfml-system

analysis.txt: gmon.out | $(EXEC)
	gprof $(EXEC) $< > $@

.PHONY: all gprof
