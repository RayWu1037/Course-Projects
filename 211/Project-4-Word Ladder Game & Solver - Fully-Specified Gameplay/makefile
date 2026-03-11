build:
	gcc -Wall -Wextra -g main.c -o ladders.exe

run_fun:
	./ladders.exe -p ON -n 4 -d dictionary.txt -s RAND -f RAND

run:
	./ladders.exe -n 5 -d dictionary.txt -s flame -f spark

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all ./ladders.exe -n 5 -d dictionary.txt -s flame -f spark

clean:
	rm -f ladders.exe
    