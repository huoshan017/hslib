INCDIR = $(PWD)/include
SRCDIR = $(PWD)/src
OBJDIR = $(PWD)/obj
LIBDIR = $(PWD)/lib
TESTDIR = $(PWD)/test

TARGET = $(LIBDIR)/hs_ds.so

INCS = $(INCDIR)/hs_array.h \
	   $(INCDIR)/hs_hashmap.h \
	   $(INCDIR)/hs_list.h \
	   $(INCDIR)/hs_ringbuffer.h

SRCS = $(SRCDIR)/hs_array.c \
	   $(SRCDIR)/hs_hashmap.c \
	   $(SRCDIR)/hs_list.c \
	   $(SRCDIR)/hs_ringbuffer.c

OBJS = $(OBJDIR)/hs_array.o \
	   $(OBJDIR)/hs_hashmap.o \
	   $(OBJDIR)/hs_list.o \
	   $(OBJDIR)/hs_ringbuffer.o

RINGBUFFER_OBJS = $(TESTDIR)/ringbuffer_test.o
RINGBUFFER_TEST_TARGET = $(TESTDIR)/ringbuffer_test


all: init $(TARGET) ringbuffer_test
ringbuffer_test: $(RINGBUFFER_TEST_TARGET)

init:
	mkdir -p $(PWD)/obj
	mkdir -p $(PWD)/lib

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc -c -fPIC -g $< -o $@  -I $(INCDIR)

$(TARGET): $(OBJS)
	gcc -shared -fPIC -o $(TARGET) $(OBJS) -I $(INCDIR)

$(RINGBUFFER_TEST_TARGET): $(RINGBUFFER_OBJS)
	gcc -Wall -o $(RINGBUFFER_TEST_TARGET) -g $(RINGBUFFER_OBJS) $(TARGET) -I $(INCDIR)

clean:
	rm -fr $(OBJS) $(TARGET) $(RINGBUFFER_OBJS) $(RINGBUFFER_TEST_TARGET)
