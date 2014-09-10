# Makefile for StoppedPionAnalysis mu2e - PlottingMacros subdir
OBJDIR=$(PWD)/obj
SRCDIR=$(PWD)/src

CC=g++
CFLAGS=-g -std=c++11 -Wall

# List here the object files to be used
OBJS=splitLine.o Coordinate.o CoordinateCollection.o HelperFunctions.o

all: $(OBJS)

%.o : $(SRCDIR)/%.cc 
	$(CC) $(CFLAGS) -I$(BASE_RELEASE) -c `root-config --glibs --cflags` -I$(BOOST_INC) $^
	@mv $@ $(OBJDIR)/

.PHONY: clean

clean:
	rm -f $(BINDIR)/* *~ 
