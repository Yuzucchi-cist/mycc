TARGET:=mycc
INCDIR:=lib
LIBS:=$(filter-out $(wildcard $(INCDIR)/*_test.c), $(wildcard $(INCDIR)/*.c))
SRCS:=main.c $(LIBS)
OBJS:=$(SRCS:%.c=%.o)
HDRS:=$(LIBS:%.c=%.h)
DEPS:=$(SRCS:%.c=%.d)
TESTS:=$(wildcard *_test.c) $(wildcard $(INCDIR)/*_test.c)

CFLAGS:=-std=c11 -g -static -I $(INCDIR)
CFLAGS+=-MD

all: $(TARGET)

$(TARGET): $(OBJS)
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

-include $(DEPS)
