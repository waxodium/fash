build: main.c
	gcc -Wall -Wextra -Werror main.c lib/* -o fash

run: main.c
	gcc -Wall -Wextra -Werror main.c lib/* -o fash
	./fash

clean:
	rm fash
