INCDIR = $(PWD)/include
SRCDIR = $(PWD)/src
OBJDIR = $(PWD)/obj
LIBDIR = $(PWD)/lib
TESTDIR = $(PWD)/test

TARGET = $(LIBDIR)/hs_db.so

INCS = $(INCDIR)/hs_db.h

SRCS = $(SRCDIR)/hs_db.c

OBJS = $(OBJDIR)/hs_db.o

DB_TEST_OBJS = $(TESTDIR)/db_test.o
DB_TEST_TARGET = $(TESTDIR)/db_test


all: init $(TARGET) $(DB_TEST_TARGET)

init:
	mkdir -p $(PWD)/obj
	mkdir -p $(PWD)/lib

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc -c -fPIC -g $< -o $@  -I $(INCDIR)

$(TARGET): $(OBJS)
	gcc -shared -fPIC -o $(TARGET) $(OBJS) -I $(INCDIR)

$(DB_TEST_TARGET): $(DB_TEST_OBJS)
	gcc -Wall -o $(DB_TEST_TARGET) -g $(DB_TEST_OBJS) $(TARGET) -I $(INCDIR)

clean:
	rm -fr $(OBJS) $(TARGET) $(DB_TEST_OBJS) $(DB_TEST_TARGET)
