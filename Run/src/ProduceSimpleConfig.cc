// C++ includes
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>

// ROOT includes
#include "TCanvas.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoPainter.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TGeoXtru.h"
#include "TGLViewer.h"
#include "TApplication.h"
#include "TLine.h"
#include "TPad.h"
#include "TString.h"
#include "TStyle.h"
#include "TText.h"
#include "TTree.h"
#include "TView.h"

// BOOST options - silence unused local typedefs warnings
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "boost/program_options.hpp"

// Utilities
#include "Utilities/inc/HelperFunctions.hh"
#include "Utilities/inc/Coordinate.hh"
#include "Utilities/inc/CoordinateCollection.hh"
#include "Utilities/inc/Table.hh"
#include "Utilities/inc/Config.hh"

using namespace std;
using namespace util;
namespace po = boost::program_options;

namespace {

  TGeoMaterial* matVacuum;
  TGeoMaterial* matAl;

  TGeoMedium* medVacuum;
  TGeoMedium* medAl;

  TGeoVolume* top;
  TGeoRotation* rot;

  bool draw_    = false;
  bool verbose_ = false;

  Config masterConfig;

}

void runJob(const vector<string>& arguments );
void constructPolygon     ( const CoordinateCollection& filename );
void constructDirtInferred(       CoordinateCollection  filename );
void constructDirtPolygon (       CoordinateCollection  filename );

//=================================================
int main(int argc, char* argv[]) {

  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("draw", po::value<bool>()->default_value(false), "draw flag [default is false]")
    ("verbose", po::value<bool>()->default_value(false), "print coordinate attributes [default is false]")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))   { cout << desc << "\n"; return 1; }
  if (vm.count("draw"))   { draw_    = vm["draw"]   .as<bool>();  }
  if (vm.count("verbose")){ verbose_ = vm["verbose"].as<bool>();  }

  // I/O to get .ccl files
  vector<string> args ( argv
                        + 1
                        + draw_
                        + verbose_
                        , argv+argc );

  TApplication theApp("App",&argc,argv);
  runJob( args );

}

//=================================================
void runJob( const vector<string>& args ) {

  const double dx = 50000;
  const double dy = 50000;
  const double dz = 50000;

  std::map< worldDir::enum_type, Coordinate::Rep<double>> worldCorners =
    {{
        {worldDir::NW,{-dx, dy}}, // NW corner
        {worldDir::NE,{ dx, dy}}, // NE "
        {worldDir::SE,{ dx,-dy}}, // SE "
        {worldDir::SW,{-dx,-dy}}  // SW "
      }};


  if ( draw_ ) {
    // Set up geometry parameters/volumes
    matVacuum = new TGeoMaterial("Vacuum",0,0,0);
    matAl     = new TGeoMaterial("Al",26.98,13,2.7);

    medVacuum = new TGeoMedium("Vacuum",1,matVacuum);
    medAl     = new TGeoMedium("Wall material",2,matAl);

    rot       = new TGeoRotation("rot",0.,-90.,-90.);

    // Make top-level volumes
    top = gGeoManager->MakeBox("TOP",medVacuum,dx,dy,dz);
    gGeoManager->SetTopVolume(top);
  }

  // Construct lower-level extruded polygons
  for ( const auto& filename : args ) {
    CoordinateCollection ccoll( filename, worldCorners );

    // Check for dirt polygon first
    if ( ccoll.volName().find("dirt.") != std::string::npos ) {
      if ( verbose_ ) std::cout << " Dirt polygon from file: " << filename << std::endl;
      constructDirtPolygon ( ccoll );
    }
    else {
      if ( verbose_ ) std::cout << " Polygon from file: " << filename << std::endl;
      constructPolygon     ( ccoll );

      if ( verbose_ ) std::cout << " Dirt inferred from file: " << filename << std::endl;
      constructDirtInferred( ccoll );
    }

  }

  // Sort master-config lists
  std::sort( masterConfig.bldgFiles.begin()   , masterConfig.bldgFiles.end()    );
  std::sort( masterConfig.dirtFiles.begin()   , masterConfig.dirtFiles.end()    );
  std::sort( masterConfig.bldgPrefixes.begin(), masterConfig.bldgPrefixes.end() );
  std::sort( masterConfig.dirtPrefixes.begin(), masterConfig.dirtPrefixes.end() );

  // Print master config file
  fstream mf;
  mf.open( "output/mu2eBuilding.txt", fstream::out );
  mf << "// Automatically produced by ProduceSimpleConfig\n\n";
  mf << "// This defines the vertical position of the hall air volume\n";
  mf << "double yOfFloorSurface.below.mu2eOrigin = -2312; // mm -(728.58684' - 721')\n\n";
  for( const auto& line : masterConfig.bldgFiles ) mf << line << "\n";
  mf << std::endl;
  mf << "vector<string> bldg.prefix.list = {\n" ;
  std::size_t i(0);
  for ( const auto& prefix : masterConfig.bldgPrefixes ) {
    mf << "  \"" << prefix << "\"";
    if ( i != masterConfig.bldgPrefixes.size()-1 ) mf << ",";
    mf << std::endl;
    ++i;
  }
  mf << "};\n\n";
  for( const auto& line : masterConfig.dirtFiles ) mf << line << "\n";
  mf << std::endl;
  mf << "vector<string> dirt.prefix.list = {\n" ;
  i=0;
  for ( const auto& prefix : masterConfig.dirtPrefixes ) {
    mf << "  \"" << prefix << "\"";
    if ( i != masterConfig.dirtPrefixes.size()-1 ) mf << ",";
    mf << std::endl;
    ++i;
  }
  mf << "};\n\n";
  mf << R"(// Local Variables:)" << std::endl;
  mf << R"(// mode:c++)"         << std::endl;
  mf << R"(// End:)"             << std::endl;
  mf.close();

  if ( !draw_ ) return;

  gGeoManager->CloseGeometry();
  gGeoManager->SetVisLevel(3);

  TCanvas c2;

  // Plotting options
  gStyle->SetCanvasPreferGL(kTRUE);

  top->Draw("ogl");
  gPad->WaitPrimitive();

}

//=================================================
void constructPolygon( const CoordinateCollection& ccoll ) {

  if (verbose_) std::cout << " Height: " << ccoll.height().at(0) << " to " << ccoll.height().at(1) << std::endl;

  ccoll.printSimpleConfigFile( masterConfig, "output/" );

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() ) {
      if ( verbose_ ) coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() );
    }
    ++counter;
  }

  if ( !draw_ ) return;

  TGeoVolume* vol = gGeoManager->MakeXtru( ccoll.volName().data(), medAl, 2);
  TGeoXtru*  poly = (TGeoXtru*)vol->GetShape();

  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );

  const double base   = ccoll.height().at(0);
  const double height = ccoll.height().at(1);

  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  if      ( ccoll.volName().find("oundation") != std::string::npos ||
            ccoll.volName().find("rench")     != std::string::npos ) vol->SetLineColor(kGray);
  else if ( ccoll.volName().find("floor.")    != std::string::npos ) vol->SetLineColor(28);
  else vol->SetLineColor(45);

  top->AddNode( vol, 1, rot );

}

//=================================================
void constructDirtInferred( CoordinateCollection ccoll ){

  const bool enoughOuterPoints = CoordinateCollection::hasOuterPoints( ccoll );
  if ( !enoughOuterPoints ) return;

  const bool boundariesAdded   = ccoll.addWorldBoundaries( verbose_ );
  if ( !boundariesAdded ) return;

  if (verbose_ ) std::cout << " Height: " << ccoll.height().at(0) << " to " << ccoll.height().at(1) << std::endl;

  ccoll.setName( "dirt."+ccoll.volName() );

  ccoll.printSimpleConfigFile( masterConfig, "output/", true );

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() && coord.isOutline() ) {
      if ( verbose_ ) coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() );
    }
    ++counter;
  }

  if ( !draw_ ) return;

  TGeoVolume* vol = gGeoManager->MakeXtru( TString(ccoll.volName()+"Dirt"), medAl, 2);
  TGeoXtru*  poly = (TGeoXtru*)vol->GetShape();

  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );

  const double base   = ccoll.height().at(0);
  const double height = ccoll.height().at(1);

  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  vol->SetLineColor(21);

  top->AddNode( vol, 1, rot );

}

//=================================================
void constructDirtPolygon( CoordinateCollection ccoll ){

  if (verbose_ ) std::cout << " Height: " << ccoll.height().at(0) << " to " << ccoll.height().at(1) << std::endl;

  ccoll.addWorldBoundaries( verbose_ );

  ccoll.printSimpleConfigFile( masterConfig, "output/" );

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() ) {
      if ( verbose_ ) coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() );
    }
    ++counter;
  }

  if ( !draw_ ) return;

  TGeoVolume* vol = gGeoManager->MakeXtru( ccoll.volName().data(), medAl, 2);
  TGeoXtru*  poly = (TGeoXtru*)vol->GetShape();

  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );

  const double base   = ccoll.height().at(0);
  const double height = ccoll.height().at(1);

  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  vol->SetLineColor(21);

  top->AddNode( vol, 1, rot );

}
