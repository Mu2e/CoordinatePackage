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
#include "TArrow.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TCut.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
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
#include "TLatex.h"
#include "TMarker.h"
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

  typedef Coordinate::FtInchPair FtInchPair;

  TGeoMaterial* matVacuum;
  TGeoMaterial* matAl;
  
  TGeoMedium* medVacuum;
  TGeoMedium* medAl;
  
  TGeoVolume* top;
  TGeoRotation* rot;

  bool OGL_;

  vector<TMarker> markers_;

}

void runJob(const vector<string>& arguments );
void constructPolygon( const string& filename );

//=================================================
int main(int argc, char* argv[]) {
  
  // Plotting options
  gStyle->SetOptStat(kFALSE);
  gStyle->SetCanvasPreferGL(kTRUE);

  // I/O
  check_argc( argc, { argv[0], "mode" "geometry files", "..." } );
  const string mode = util::check_argv( argv[1], {"pad","ogl"} );
  OGL_ = mode.find("ogl") != string::npos;
  vector<string> args ( argv+2, argv+argc );
  
  TApplication theApp("App",&argc,argv);
  runJob( args );
  theApp.Run();

}

//=================================================
void runJob( const vector<string>& args ) {
  
  TCanvas c2;
  for ( const auto& file : args ) {
    constructPolygon( file );
  }
  gGeoManager->CloseGeometry();
  gGeoManager->SetVisLevel(3);

  if ( OGL_ ) {
    top->Draw("ogl");
    gPad->WaitPrimitive();
  }
  else {
    top->Draw();

    gPad->GetView()->SetAutoRange();
  }

  c2.SaveAs("test.eps");
}

//=================================================
void constructPolygon( const string& filename ) {

  static int callCounter(0);
  callCounter++;

  std::cout << " Polygon from file: " << filename << std::endl;

  CoordinateCollection ccoll( filename );
  
  std::cout << " Height: " << ccoll.height().at(0) << " to " << ccoll.height().at(1) << std::endl;

  vector<double> xPos, yPos;
  unsigned counter(0);
  for( const auto& coord : ccoll.coordinates() ) {
    if ( counter != 0 && coord.drawFlag() ) {
      coord.print();
      xPos.push_back( coord.x() );
      yPos.push_back( coord.y() );
      markers_.push_back( TMarker(xPos.back(),yPos.back(),20) );
    }
    ++counter;
  }

  if ( callCounter == 1 ) {
    matVacuum = new TGeoMaterial("Vacuum",0,0,0);
    matAl     = new TGeoMaterial("Al",26.98,13,2.7);
    
    medVacuum = new TGeoMedium("Vacuum",1,matVacuum);
    medAl     = new TGeoMedium("Wall material",2,matAl);

    rot       = new TGeoRotation("rot",0.,-90.,-90.);

    top = gGeoManager->MakeBox("TOP",medVacuum,50000,50000,50000);
    gGeoManager->SetTopVolume(top);
  }

  TGeoVolume* vol = gGeoManager->MakeXtru( TString::Format("xtru%d",callCounter) ,medAl, 2);
  TGeoXtru* poly = (TGeoXtru*)vol->GetShape();
  
  poly->DefinePolygon( xPos.size(), &xPos.front(), &yPos.front() );

  const double base   = OGL_ ? ccoll.height().at(0) : 0;
  const double height = OGL_ ? ccoll.height().at(1) : 1;

  poly->DefineSection( 0,base  ,0,0,1 );
  poly->DefineSection( 1,height,0,0,1 );

  if ( filename.find("oundation") != std::string::npos || 
       filename.find("rench")     != std::string::npos )  vol->SetLineColor(kGray);
  else if ( filename.find("floor") != std::string::npos ) vol->SetLineColor(28);
  else vol->SetLineColor(45);

  top->AddNode( vol, 1, rot );

}
