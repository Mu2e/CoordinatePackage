# CoordinatePackage
Original Mu2e hall geometry generator

The instructions below work for both AL9/spack and
in an SL7 container with UPS.


mu2einit
git clone https://github.com/Mu2e/CoordinatePackage
muse setup

cd CoordinatePackage
source setup.sh
make all
./Run/bin/ProduceSimpleConfig geom/*

The output files are in output/
Compare these to Offline/Mu2e/G4/geom/bldg.


The original instructions from the README file are copied below.
I have not found the source for TestGeometry or managed to get
the ogl or pad options to work for ProduceSimpleConfig

--- Original instructions start here:
To setup this package, do:

  setup mu2e
  source setup.sh

To build the package, do

  make all

The code is run by doing:

  TestGeometry ogl geom*.txt

where the arguments received are the plotting option ("pad" or "ogl"),
and list of txt files of coordinates.


