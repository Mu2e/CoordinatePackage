#
OBJDIR=$(PWD)/obj
SRCDIR=$(PWD)/src

CC=g++
CFLAGS=-g -std=c++20 -Wall

# List here the object files to be used
OBJS=splitLine.o Coordinate.o CoordinateCollection.o HelperFunctions.o

all: $(OBJS)

%.o : $(SRCDIR)/%.cc
	echo "Base release " $(BASE_RELEASE)
	$(CC) -c $(CFLAGS) -I$(BASE_RELEASE) -I$(ROOT_INC) -I$(BOOST_INC) $^
	@mv $@ $(OBJDIR)/

.PHONY: clean

clean:
	rm -f $(BINDIR)/* *~
