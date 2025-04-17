lode_runner: lode_runner.o player.c
	gcc -Wall -Wextra -fsanitize=undefined,address -o lode_runner lode_runner.o player.c -lm