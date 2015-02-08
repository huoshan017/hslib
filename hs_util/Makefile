INCDIR = $(PWD)/include
SRCDIR = $(PWD)/src
OBJDIR = $(PWD)/obj
LIBDIR = $(PWD)/lib
TESTDIR = $(PWD)/test

TARGET = $(LIBDIR)/hs_util.so

INCS = $(INCDIR)/hs_rc4.h \
	   $(INCDIR)/hs_odbc.h

SRCS = $(SRCDIR)/hs_rc4.c \
	   $(SRCDIR)/hs_odbc.c

OBJS = $(OBJDIR)/hs_rc4.o \
	   $(OBJDIR)/hs_odbc.o

RC4_TEST_SRCS = $(TESTDIR)/rc4_test.c
RC4_TEST_OBJS = $(TESTDIR)/rc4_test.o
RC4_TEST_TARGET = $(TESTDIR)/rc4_test

ODBC_TEST_SRCS = $(TESTDIR)/odbc_test.c
ODBC_TEST_OBJS = $(TESTDIR)/odbc_test.o
ODBC_TEST_TARGET = $(TESTDIR)/odbc_test


all: init $(TARGET) $(RC4_TEST_TARGET) $(ODBC_TEST_TARGET)

init:
	mkdir -p $(PWD)/obj
	mkdir -p $(PWD)/lib

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc -c -fPIC -g $< -o $@  -I $(INCDIR)

$(TARGET): $(OBJS)
	gcc -shared -fPIC -o $(TARGET) $(OBJS) -I $(INCDIR)

$(RC4_TEST_OBJS): $(TESTDIR)/%.o: $(TESTDIR)/%.c
	gcc -c -fPIC -g $< -o $@  -I $(INCDIR)

$(RC4_TEST_TARGET): $(RC4_TEST_OBJS)
	gcc -Wall -o $(RC4_TEST_TARGET) -g $(RC4_TEST_OBJS) $(TARGET) -lodbc  -I $(INCDIR)

$(ODBC_TEST_OBJS): $(TESTDIR)/%.o: $(TESTDIR)/%.c
	gcc -c -fPIC -g $< -o $@  -I $(INCDIR)

$(ODBC_TEST_TARGET): $(ODBC_TEST_OBJS)
	gcc -Wall -o $(ODBC_TEST_TARGET) -g $(ODBC_TEST_OBJS) $(TARGET) -lodbc -I $(INCDIR)

clean:
	rm -fr $(OBJS) $(TARGET) $(RC4_TEST_OBJS) $(RC4_TEST_TARGET) $(ODBC_TEST_OBJS) $(ODBC_TEST_TARGET)
