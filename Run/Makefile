#
OBJDIR=$(PWD)/obj
BINDIR=$(PWD)/bin
SRCDIR=$(PWD)/src

CC=g++
CFLAGS=-g -std=c++11 -Wall

# List here the executables to be run
all: $(BINDIR)/ProduceSimpleConfig

$(BINDIR)/ProduceSimpleConfig : \
	$(SRCDIR)/ProduceSimpleConfig.cc \
	$(BASE_RELEASE)/Utilities/obj/splitLine.o \
	$(BASE_RELEASE)/Utilities/obj/Coordinate.o \
	$(BASE_RELEASE)/Utilities/obj/CoordinateCollection.o \
	$(BASE_RELEASE)/Utilities/obj/HelperFunctions.o \
	$(BOOST_LIB)/libboost_program_options.so
	$(CC) $(CFLAGS) -o $@ `root-config --cflags --glibs` -lRGL -lGeom -I $(BASE_RELEASE) -I $(BOOST_INC) $^

.PHONY: clean

clean:
	rm -f $(OBJDIR)/* $(BINDIR)/* *~
