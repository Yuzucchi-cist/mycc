TARGET:=mycc
INCDIR:=lib
LIBS:=$(filter-out $(wildcard $(INCDIR)/*_test.c), $(wildcard $(INCDIR)/*.c))
SRCS:=$(wildcard *.c) $(LIBS)
OBJS:=$(SRCS:%.c=%.o)
HDRS:=$(LIBS:%.c=%.h)
DEPS:=$(SRCS:%.c=%.d)
TESTS:=$(wildcard *_test.c) $(wildcard $(INCDIR)/*_test.c)

CFLAGS:=-std=c11 -g -static -I $(INCDIR)

all: $(TARGET)

$(TARGET): $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

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
