CC:=gcc
OBJS:=./src/main.c src/alias.c
FLAGS:=-Wall -Wextra -Wpedantic

./bin/main: $(OBJS)
	gcc $(FLAGS) $(OBJS) -o ./bin/main

run: ./bin/main
	./bin/main

clean:
	rm -f ./bin/*