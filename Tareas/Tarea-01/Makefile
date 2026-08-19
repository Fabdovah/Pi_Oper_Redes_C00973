# Linux Makefile
# Socket classes
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c
DEPS := Socket.h VSocket.h
TARGETS := ipv4-test.out ipv6-test.out

all: $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

ipv4-test.out: ipv4-test.o Socket.o VSocket.o
	$(CXX) -g ipv4-test.o Socket.o VSocket.o -o $@

ipv6-test.out: ipv6-test.o Socket.o VSocket.o
	$(CXX) -g ipv6-test.o Socket.o VSocket.o -o $@

clean:
	@rm -f *.o *.out
