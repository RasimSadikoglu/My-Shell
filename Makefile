CC:=gcc
OBJS:=./src/main.c src/alias.c src/process_handler.c
FLAGS:=-Wall -Wextra -Wpedantic

./bin/main: $(OBJS) ./bin/ src/test.c
	gcc $(FLAGS) $(OBJS) -o ./bin/main
	gcc src/test.c -o bin/test

./bin/:
	mkdir bin

run: ./bin/main
	./bin/main

clean:
	rm -f ./bin/*