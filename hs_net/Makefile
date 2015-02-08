INCDIR = $(PWD)/include
SRCDIR = $(PWD)/src
OBJDIR = $(PWD)/obj
LIBDIR = $(PWD)/lib
TESTDIR = $(PWD)/test
CONNECTORTESTDIR = $(PWD)/test
ACCEPTORTESTDIR = $(PWD)/test
HSDSDIR = $(PWD)../hs_ds
HSDSINCDIR = $(PWD)/../hs_ds/include
HSDSLIBDIR = $(PWD)/../hs_ds/lib

TARGET = $(LIBDIR)/hs_net.so

INCS = $(INCDIR)/hs_recvbuff.h \
	   $(INCDIR)/hs_sendbuff.h \
	   $(INCDIR)/hs_net_server.h \
	   $(INCDIR)/hs_net_agent.h \
	   $(INCDIR)/hs_net_connector.h \
	   $(INCDIR)/hs_net_acceptor.h \
	   $(INCDIR)/hs_net_util.h

SRCS = $(SRCDIR)/hs_recvbuff.c \
	   $(SRCDIR)/hs_sendbuff.c \
	   $(SRCDIR)/hs_net_server.c \
	   $(SRCDIR)/hs_net_agent.c \
	   $(SRCDIR)/hs_net_connector.c \
	   $(SRCDIR)/hs_net_acceptor.c \
	   $(SRCDIR)/hs_net_util.c

OBJS = $(OBJDIR)/hs_recvbuff.o \
	   $(OBJDIR)/hs_sendbuff.o \
	   $(OBJDIR)/hs_net_server.o \
	   $(OBJDIR)/hs_net_agent.o \
	   $(OBJDIR)/hs_net_connector.o \
	   $(OBJDIR)/hs_net_acceptor.o \
	   $(OBJDIR)/hs_net_util.o

TEST_SRCS = $(TESTDIR)/server.c
TEST_OBJS = $(TESTDIR)/server.o
TEST_TARGET = $(TESTDIR)/my_server

CONNECTOR_TEST_SRCS = $(CONNECTORTESTDIR)/connect_test.c
CONNECTOR_TEST_OBJS = $(CONNECTORTESTDIR)/connect_test.o
CONNECTOR_TEST_TARGET = $(CONNECTORTESTDIR)/connect_test

ACCEPTOR_TEST_SRCS = $(ACCEPTORTESTDIR)/acceptor_test.c
ACCEPTOR_TEST_OBJS = $(ACCEPTORTESTDIR)/acceptor_test.o
ACCEPTOR_TEST_TARGET = $(ACCEPTORTESTDIR)/acceptor_test

all: init $(TARGET) server_test connect_test acceptor_test
server_test: $(TEST_TARGET)
connect_test: $(CONNECTOR_TEST_TARGET)
acceptor_test: $(ACCEPTOR_TEST_TARGET)

init:
	mkdir -p $(PWD)/obj
	mkdir -p $(PWD)/lib

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c 
	gcc -c -fPIC -g $< -o $@ -I $(INCDIR) -I $(HSDSINCDIR)

$(TARGET): $(OBJS)
	gcc -shared -fPIC -o $(TARGET) $(OBJS) 

$(TEST_OBJS): $(TESTDIR)/%.o: $(TESTDIR)/%.c
	gcc -c -fPIC -g $< -o $@ -I $(INCDIR) -I $(HSDSINCDIR)

$(TEST_TARGET): $(TEST_OBJS)
	gcc -Wall -o $(TEST_TARGET) -g $(TEST_OBJS) $(LIBDIR)/hs_net.so $(HSDSLIBDIR)/hs_ds.so

$(CONNECTOR_TEST_OBJS): $(CONNECTORTESTDIR)/%.o: $(CONNECTORTESTDIR)/%.c
	gcc -c -fPIC -g $< -o $@ -I $(INCDIR) -I $(HSDSINCDIR)

$(CONNECTOR_TEST_TARGET): $(CONNECTOR_TEST_OBJS)
	gcc -Wall -o $(CONNECTOR_TEST_TARGET) -g $(CONNECTOR_TEST_OBJS) $(LIBDIR)/hs_net.so $(HSDSLIBDIR)/hs_ds.so -lssl -lcrypto

$(ACCEPTOR_TEST_OBJS): $(ACCEPTORTESTDIR)/%.o: $(ACCEPTORTESTDIR)/%.c
	gcc -c -fPIC -g $< -o $@ -I $(INCDIR) -I $(HSDSINCDIR)

$(ACCEPTOR_TEST_TARGET): $(ACCEPTOR_TEST_OBJS)
	gcc -Wall -o $(ACCEPTOR_TEST_TARGET) -g $(ACCEPTOR_TEST_OBJS) $(LIBDIR)/hs_net.so $(HSDSLIBDIR)/hs_ds.so

clean:
	rm -fr $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET) $(CONNECTOR_TEST_OBJS) $(CONNECTOR_TEST_TARGET) $(ACCEPTOR_TEST_OBJS) $(ACCEPTOR_TEST_TARGET)
