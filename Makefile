CC      := gcc
CXX     := g++
INCLUDE := -Iinclude
LIBS    := 
CARGS   := $(INCLUDE) -ggdb -Wall -Wextra -pedantic #-Werror

#submodules
cmd_obj += cmdargs.o

#executables
yc_obj += exec/yc/main.o $(cmd_obj)

all: bin/yc

bin/yc: $(addprefix obj/, $(yc_obj))
	@mkdir -p $(dir ./$@)
	$(CXX) $(CARGS) -o ./$@ $^ $(LIBS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CARGS) -o $@ $^

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CARGS) -o $@ $^

clean:
	rm -rf ./bin
	rm -rf ./obj