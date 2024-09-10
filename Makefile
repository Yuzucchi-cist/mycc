TARGET:=mycc
INCDIR:=./lib
SRCS:=$(wildcard *.c) $(wildcard $(INCDIR)/*.c)
OBJS:=$(SRCS:%.c=%.o)
DEPS:=$(SRCS:%.c=%.d)

CFLAGS=-std=c11 -g -static -I $(INCDIR)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.c.o:

test: $(TARGET)
	./test.sh

debug: $(TARGET)
	gdb $^

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) tmp*

.PHONY: test debug clean
