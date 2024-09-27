TARGET:=mycc
INCDIR:=lib
LIBS:=$(filter-out $(wildcard $(INCDIR)/*_test.c), $(wildcard $(INCDIR)/*.c))
SRCS:=main.c $(LIBS)
OBJS:=$(SRCS:%.c=%.o)
HDRS:=$(LIBS:%.c=%.h)
DEPS:=$(SRCS:%.c=%.d)
UTESTS:=$(wildcard *_test.c) $(wildcard $(INCDIR)/*_test.c)
TESTDIR:=test
TESTS:=$(filter-out $(tests), $(wildcard $(TESTDIR)/*.c))

CFLAGS:=-std=c11 -g -static -I $(INCDIR)
CFLAGS+=-MD

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

.c.o:

test: $(TARGET)
	./$< $(TESTDIR)/tests > tmp.s
	$(CC) -static -g -o tmp tmp.s $(TESTS)
	./tmp

$(UTESTS:%.c=%): $(UTESTS:%.c=%.o) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(filter-out main.o, $(OBJS))
utest: $(UTESTS:%.c=%)
	./$^

debug: $(TARGET)
	gdb $^

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) tmp*

.PHONY: test debug clean

-include $(DEPS)
