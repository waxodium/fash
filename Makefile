build: main.c
	gcc -Wall -Wextra -Werror main.c -o fcsh

run: main.c
	gcc -Wall -Wextra -Werror main.c -o fcsh
	./fcsh

clean:
	rm fcsh
