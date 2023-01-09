CC = gcc

CFLAGS = -Wextra -Wall -Wrestrict -std=c99 -s  -O2 -mtune=native -march=native
TFLAGS = -Wextra -Wall -Wrestrict -lasan -std=c99 -g  -O2 -mtune=native -march=native
PFlAGS = -Wextra -Wall -Wrestrict -std=c99 -pg -O2 -mtune=native -march=native

SRCS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.c' -print)

main: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o "$@"

debug: $(SRCS)
	$(CC) $(TFLAGS) $(SRCS) -o "$@"

clean:
	rm -f main main-debug
	
disasm:
	$(CC) $(CFLAGS) -masm=intel -S $(SRCS)
