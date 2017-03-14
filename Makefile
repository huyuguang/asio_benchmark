#CXX11_HOME = /usr/local/gcc6

CXX11_HOME = /usr

ASIO_HOME = ./asio_alone

CXXFLAGS := \
 -g3 \
 -fPIC \
 -std=c++11 \
 -Wall \
 -Wextra \
 -gdwarf-2 \
 -gstrict-dwarf \
 -Wno-unused-parameter \
 -Wno-parentheses \
 -Wdeprecated-declarations \
 -fmerge-all-constants  \
 -I $(CXX11_HOME)/include \
 -I $(ASIO_HOME) \
 -I.

RELEASE_FLAGS := \
 -O3 \
 -DDTREE_ICASE \
 -DNDEBUG \
 -DLINUX \

DEBUG_FLAGS := \
 -O0 \
 -D_DEBUG \
 -DLINUX \
 
#LDFLAGS += \
# -static-libstdc++ -static-libgcc \
# -fmerge-all-constants \
# -L${CXX11_HOME}/lib64 \
# -L./

 LDFLAGS += \
 -fmerge-all-constants \
 -L${CXX11_HOME}/lib64 \
 -L./

LIBS := \
 -lpthread \
 -lrt


DIR := . 

SRC := $(foreach d, $(DIR), $(wildcard $(d)/*.cpp))



RELEASE_OBJ := $(patsubst %.cpp, %.o, $(SRC))

DEBUG_OBJ := $(patsubst %.cpp, %.d.o, $(SRC))



CXX := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/g++

CC := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/gcc



all: asio_test 

%.o : %.cpp
	$(CXX) -c $^ $(CXXFLAGS) $(RELEASE_FLAGS) -o $@


%.d.o : %.cpp
	$(CXX) -c $^ $(CXXFLAGS) $(DEBUG_FLAGS) -o $@



asio_test : $(RELEASE_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)


clean:
	find . -regex "\(.*\.o\|.*\.exe\)" | xargs rm

.PHONY :
