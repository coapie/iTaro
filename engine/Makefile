
CXX := g++

SEASTAR := /home/fangyou/workspace/seastar
CXXFLAGS := -Wall -O2 -g -std=c++1y -I$(SEASTAR) -I$(SEASTAR)/apps

LDFLAGS := -L$(SEASTAR)/build/release -Wl,--whole-archive -lseastar -Wl,--no-whole-archive -pthread  -laio -lboost_program_options -lboost_system -lstdc++ -lm -lboost_unit_test_framework -lboost_thread -lcryptopp -lrt 

obj-oxx := proto.o session.o server.o main.o

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<


all : $(obj-oxx)
	$(CXX)  -o meson $(obj-oxx) $(LDFLAGS)


clean :
	rm -f $(obj-oxx) meson

