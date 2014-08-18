# Makefile for StoppedPionAnalysis mu2e - Analysis subdir
OBJDIR=$(PWD)/obj
BINDIR=$(PWD)/bin
SRCDIR=$(PWD)/src

CC=g++
CFLAGS=-g -std=c++11 -Wall

# List here the executables to be run
all: $(BINDIR)/TestGeometry

$(BINDIR)/TestGeometry : $(SRCDIR)/TestGeometry.cc $(BASE_RELEASE)/Utilities/obj/splitLine.o $(BASE_RELEASE)/Utilities/obj/Coordinate.o $(BASE_RELEASE)/Utilities/obj/CoordinateCollection.o  $(BASE_RELEASE)/Utilities/obj/HelperFunctions.o
	$(CC) $(CFLAGS) -o $@ `root-config --cflags --glibs` -lRGL -lGeom -I $(BASE_RELEASE) $^

.PHONY: clean

clean:
	rm -f $(BINDIR)/* *~ 
