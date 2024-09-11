TARGET:=mycc
INCDIR:=lib
SRCS:=$(wildcard *.c) $(filter-out $(INCDIR)/codegen_test.c, $(wildcard $(INCDIR)/*.c))
OBJS:=$(SRCS:%.c=%.o)
DEPS:=$(SRCS:%.c=%.d)
TESTS:=$(wildcard *_test.c) $(wildcard $(INCDIR)/*_test.c)

CFLAGS:=-std=c11 -g -static -I $(INCDIR)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.c.o:

test: $(TARGET)
	./test.sh

$(TESTS:%.c=%): $(TESTS:%.c=%.o) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(filter-out main.o, $(OBJS))
utest: $(TESTS:%.c=%)
	./$^

debug: $(TARGET)
	gdb $^

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) tmp*

.PHONY: test debug clean
