# TARGET NAMES
TARGET = VASimVis

# DIRECTORIES
VASIM = VASim
MNRL = $(VASIM)/libs/MNRL/C++
JSON11 = $(MNRL)/lib/json11
JSON = $(MNRL)/lib/valijson/thirdparty/nlohmann-json-1.1.0
PUGI = $(VASIM)/libs/pugixml/src
WT=-lwthttp -lwt -lboost_random -lboost_regex -lboost_signals -lboost_system -lboost_thread -lboost_filesystem -lboost_program_options -lboost_date_time
LIBS=/usr/local/

# DEPENDENCIES
LIBVASIM = $(VASIM)/libvasim.a
LIBMNRL = $(MNRL)/libmnrl.a
JSON11_HPP = $(JSON11)/json11.hpp

# FLAGS
CXXFLAGS= -O3 -Iinclude -I$(LIBS)/include -L$(LIBS)/lib $(WT) -I$(MNRL)/include -I$(VASIM)/include -I$(PUGI) -I$(JSON) -I$(JSON11) -std=c++11

CC=g++-5


all:
	$(CC) -o $(TARGET) VASimVis.cc $(LIBVASIM) $(LIBMNRL) $(CXXFLAGS)
