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

// Utilities
#include "Utilities/inc/HelperFunctions.hh"
#include "Utilities/inc/Coordinate.hh"
#include "Utilities/inc/CoordinateCollection.hh"
#include "Utilities/inc/Table.hh"

using namespace std;
using namespace util;

namespace {

  const double dirtGradeAboveFloor = 10363.2;

  typedef Coordinate::FtInchPair FtInchPair;

  TGeoMaterial* matVacuum;
  TGeoMaterial* matAl;
  
  TGeoMedium* medVacuum;
  TGeoMedium* medAl;
  
  TGeoVolume* top;
  TGeoRotation* rot;

}

void runJob(const vector<string>& arguments );
void constructPolygon     ( const CoordinateCollection& filename );
void constructDirtInferred(       CoordinateCollection  filename );
void constructDirtPolygon (       CoordinateCollection  filename );

//=================================================
int main(int argc, char* argv[]) {
  
  // Plotting options
  gStyle->SetOptStat(kFALSE);
  gStyle->SetCanvasPreferGL(kTRUE);

  // I/O
  check_argc( argc, { argv[0], "geometry files", "..." } );
  vector<string> args ( argv+1, argv+argc );
  
  TApplication theApp("App",&argc,argv);
  runJob( args );
  theApp.Run();

}

//=================================================
void runJob( const vector<string>& args ) {
  
  // Set up geometry parameters/volumes
  matVacuum = new TGeoMaterial("Vacuum",0,0,0);
  matAl     = new TGeoMaterial("Al",26.98,13,2.7);
  
  medVacuum = new TGeoMedium("Vacuum",1,matVacuum);
  medAl     = new TGeoMedium("Wall material",2,matAl);
  
  rot       = new TGeoRotation("rot",0.,-90.,-90.);
  
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
  
  
  // Make top-level volumes
  top = gGeoManager->MakeBox("TOP",medVacuum,dx,dy,dz);
  gGeoManager->SetTopVolume(top);
  
  // Construct lower-level extruded polygons
  for ( const auto& filename : args ) {
    CoordinateCollection ccoll( filename, worldCorners );
    
    // Check for dirt polygon first
    if ( ccoll.volName().find("DirtPolygon_") != std::string::npos ) {
      std::cout << " Dirt polygon from file: " << filename << std::endl;  
      constructDirtPolygon ( ccoll );
    }
    else {
      std::cout << " Polygon from file: " << filename << std::endl;
      constructPolygon     ( ccoll );
      
      std::cout << " Dirt inferred from file: " << filename << std::endl;  
      constructDirtInferred( ccoll );
    }

  }
  gGeoManager->CloseGeometry();
  gGeoManager->SetVisLevel(3);

  TCanvas c2;
  
  top->Draw("ogl");
  gPad->WaitPrimitive();
  
}

//=================================================
void constructPolygon( const CoordinateCollection& ccoll ) {

  std::cout << " Height: " << ccoll.height().at(0) << " to " << ccoll.height().at(1) << std::endl;

  ccoll.printSimpleConfigFile( "geom/simpleConfig_"+ccoll.volName()+".txt" );

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() ) {
      coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() );
    }
    ++counter;
  }

  TGeoVolume* vol = gGeoManager->MakeXtru( ccoll.volName().data(), medAl, 2);
  TGeoXtru*  poly = (TGeoXtru*)vol->GetShape();
  
  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );

  const double base   = ccoll.height().at(0);
  const double height = ccoll.height().at(1);

  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  if      ( ccoll.volName().find("oundation") != std::string::npos || 
            ccoll.volName().find("rench")     != std::string::npos ) vol->SetLineColor(kGray);
  else if ( ccoll.volName().find("floor")     != std::string::npos ) vol->SetLineColor(28);
  else vol->SetLineColor(45);

  top->AddNode( vol, 1, rot );

}

//=================================================
void constructDirtInferred( CoordinateCollection ccoll ){

  const bool enoughOuterPoints = CoordinateCollection::hasOuterPoints( ccoll );
  if ( !enoughOuterPoints ) return;

  const bool boundariesAdded   = ccoll.addWorldBoundaries();
  if ( !boundariesAdded ) return;

  ccoll.printSimpleConfigFile( "geom/simpleConfig_dirt_"+ccoll.volName()+".txt", true );

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() && coord.isOutline() ) {
      coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() );
    }
    ++counter;
  }
  
  TGeoVolume* vol = gGeoManager->MakeXtru( TString(ccoll.volName()+"_dirt"), medAl, 2);
  TGeoXtru*  poly = (TGeoXtru*)vol->GetShape();

  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );

  const double base   = ccoll.height().at(0);
  const double height = ccoll.height().at(1) > dirtGradeAboveFloor ? dirtGradeAboveFloor : ccoll.height().at(1);

  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  vol->SetLineColor(21);

  top->AddNode( vol, 1, rot );

}

//=================================================
void constructDirtPolygon( CoordinateCollection ccoll ){

  std::cout << " Height: " << ccoll.height().at(0) << " to " << ccoll.height().at(1) << std::endl;

  ccoll.addWorldBoundaries();
  
  ccoll.printSimpleConfigFile( "geom/simpleConfig_dirt_"+ccoll.volName()+".txt" );

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() ) {
      coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() ); 
    }
    ++counter;
  }
  
  TGeoVolume* vol = gGeoManager->MakeXtru( ccoll.volName().data(), medAl, 2);
  TGeoXtru*  poly = (TGeoXtru*)vol->GetShape();
  
  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );
  
  const double base   = ccoll.height().at(0);
  const double height = ccoll.height().at(1) > dirtGradeAboveFloor ? dirtGradeAboveFloor : ccoll.height().at(1);
  
  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  vol->SetLineColor(21);

  top->AddNode( vol, 1, rot );

}
