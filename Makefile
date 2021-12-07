CC:=gcc
OBJS:=./src/main.c src/alias.c
FLAGS:=-Wall -Wextra -Wpedantic

./bin/main: $(OBJS) ./bin/
	gcc $(FLAGS) $(OBJS) -o ./bin/main

./bin/:
	mkdir bin

run: ./bin/main
	./bin/main

clean:
	rm -f ./bin/*