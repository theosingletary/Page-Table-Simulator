CC := clang -g
CFLAGS := -c
LDFLAGS := -lm

libmlpt.a: libmlpt.a(functions.o)
	ranlib libmlpt.a

all: ptsim

ptsim: main.o functions.o libmlpt.a
	$(CC) -o $@ $^ $(LDFLAGS)

functions.o: functions.c mlpt.h config.h
	$(CC) $(CFLAGS) $(CFLAGS) functions.c

main.o: main.c mlpt.h config.h
	$(CC) $(CFLAGS) $(CFLAGS) main.c

clean:
	rm --force ptsim main.o functions.o libmlpt.a
