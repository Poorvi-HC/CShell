clean:
	rm -r ./obj
	rm ./cshell
	rm ./history.txt

all: src/cshell.c src/utilities.c src/commands/*.c
	mkdir -p obj
	g++ -g src/cshell.c src/utilities.c src/commands/*.c -o cshell
	chmod +x cshell

