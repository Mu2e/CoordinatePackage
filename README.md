# CoordinatePackage
Original Mu2e hall geometry generator

Instructions to build and run in an SL7 container.

mu2einit
git clone https://github.com/Mu2e/CoordinatePackage
muse setup

cd CoordinatePackage
source setup.sh
make all
./Run/bin/ProduceSimpleConfig geom/*


--- Original instructions below -- these do not work.
To setup this package, do:

  setup mu2e
  source setup.sh

To build the package, do

  make all

The code is run by doing:

  TestGeometry ogl geom*.txt

where the arguments received are the plotting option ("pad" or "ogl"),
and list of txt files of coordinates.


