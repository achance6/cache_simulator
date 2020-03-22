CC = g++
CFLAGS = -std=c++11 -Wall -Wextra --pedantic -g

%.o : %.cpp
	$(CC) $(CFLAGS) -c $<

csim: main.o
	$(CC) -o $@ main.o cache_commands.o io.o

clean:
	rm -f *.o csim

