# Linux Makefile
# Socket classes
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c
DEPS := Socket.h
TARGETS := ipv4-test.out ipv6-test.out server.out client.out

ALL := $(TARGETS)

all: $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

ipv4-test.out: ipv4-test.o Socket.o VSocket.o
	$(CXX) -g ipv4-test.o Socket.o VSocket.o -o $@

ipv6-test.out: ipv6-test.o Socket.o VSocket.o
	$(CXX) -g ipv6-test.o Socket.o VSocket.o -o $@

# Cliente/servidor propios (protocolo ECCIP/1.0), ver PROTOCOLO.md
client.out: client.o Socket.o VSocket.o
	$(CXX) -g client.o Socket.o VSocket.o -o $@

server.o: server.cc
	$(CXX) $(CXXFLAGS) -pthread server.cc

server.out: server.o
	$(CXX) -g -pthread server.o -o $@

clean:
	@rm -f *.o *.out

