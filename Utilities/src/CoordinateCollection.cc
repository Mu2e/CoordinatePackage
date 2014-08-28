// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel

#include "Utilities/inc/CoordinateCollection.hh"
#include "Utilities/inc/Table.hh"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>
#include <stdexcept>

using namespace worldDir;

namespace util {
  
  //=========================================================================
  CoordinateCollection::CoordinateCollection( const std::string& inputFile,
                                              const std::map<enum_type,Rep<double>>& worldCorners )
    : inputFile_( inputFile )
    , worldCorners_( worldCorners ) 
  {
    Table<1> inputTable = loadTable<1>( inputFile_ );
    unsigned counter(0);
    for( const auto& row : inputTable.rawTable() ) {
      if      ( counter == 0 ) volName_ = assignVolName( row.first );
      else if ( counter == 1 ) height_  = assignHeight ( row.first );
      else {
        Coordinate coordStr ( row.first );
        check_and_push_back( coordStr );
      }
      ++counter;
    }
  }
  
  //=========================================================================
  std::string CoordinateCollection::assignVolName( const std::string& inputString ) {
    if ( inputString.find("VolName)") == std::string::npos ) 
      throw std::runtime_error("Volume name not specified!  First label must be:\n \"VolName)..a..string..\"");

    const std::size_t delimPos = inputString.find(")");
    return inputString.substr(delimPos+1, inputString.length() );
  }

  //=========================================================================
  Coordinate::Rep<double> CoordinateCollection::assignHeight( const std::string& inputString ) {
    if ( inputString.find("Height)") == std::string::npos ) 
      throw std::runtime_error("Height of collection not specified!  Second label must be:\n \"Height)...,...\"");

    return Coordinate( inputString ).getCoordRel();
  }

  //=========================================================================
  void CoordinateCollection::check_and_push_back( Coordinate& coord ) {

    const auto insertTest = keyList_.insert( coord.label() );

    if ( !insertTest.second ) {
      throw std::runtime_error("Label << "+coord.label()+" >> already used!");
    }
    
    // Get reference label
    if ( coordList_.empty() ) coord.setRefLabel("*");
    else if ( coord.refLabel().empty() ) coord.setRefLabel( coordList_.back().refLabel() );
    
    // Get rotation wrt reference
    if ( coordList_.empty() ) coord.setRotation(0.);
    else if ( coord.rot() < -360 ) coord.setRotation( coordList_.back().rot() );

    if ( keyList_.find( coord.refLabel() ) == std::end(keyList_) ) {
      throw std::runtime_error("Reference label << "+coord.refLabel()+" >> does not exist yet!");
    }
    
    // Get origin
    if ( !coordList_.empty() ) {
      const auto& origin = getReferenceCoordinate( coord.refLabel() );
      const double phi   = coord.rot()*M_PI/180;
      coord.setAbsX( origin.x() + std::cos( phi )*coord.getCoordRel().at(0) - std::sin( phi )*coord.getCoordRel().at(1) );
      coord.setAbsY( origin.y() + std::sin( phi )*coord.getCoordRel().at(0) + std::cos( phi )*coord.getCoordRel().at(1) );
    }
    else {
      coord.setAbsX( coord.getCoordRel().at(0) );
      coord.setAbsY( coord.getCoordRel().at(1) );
    }

    // Add coordinate to boundary list
    if ( coord.worldBoundary() != worldDir::none ) {
      if ( boundaryList_.size() == 2 ) {
        throw std::runtime_error("Cannot add coordinate << " +coord.inputString()+ " >> to boundaryList (already full)" );
      }
      else { 
        boundaryList_.push_back( coord );        
      }
    }
      
    coordList_.push_back( coord );

  }

  //============================================
  Coordinate CoordinateCollection::getReferenceCoordinate( const std::string& refLabel ) const {
    
    auto match  = std::find_if( std::begin( coordList_ ), std::end( coordList_), 
				[&](const Coordinate& cs){ return cs.label().find( refLabel ) != std::string::npos; } );

    if ( match == std::end( coordList_ ) )
      throw std::runtime_error("Reference coordinate << "+refLabel+" >> notFound!");

    return *match;    
  }

  //============================================
  bool CoordinateCollection::addWorldBoundaries() {
    
    if ( boundaryList_.size() != 2 ) {
      std::cout << " No boundary points present " << std::endl;
      return false;
    }

    const auto& boundPt1 = boundaryList_.at(0);
    const auto& boundPt2 = boundaryList_.at(1);
    
    checkBoundaryPairForCongruency( boundPt1, boundPt2 );

    // Add wall coordinates
    std::ostringstream os1; os1 << boundPt1.label() << "_to_" << boundPt1.worldBoundary();
    std::ostringstream os2; os2 << boundPt2.label() << "_to_" << boundPt2.worldBoundary();

    Coordinate c1 ( getWallCoordinate( boundPt1, os1.str() ) );
    Coordinate c2 ( getWallCoordinate( boundPt2, os2.str() ) );

    // Inserting in reverse order to agree with handedness of solid
    // definition
    //
    // - Don't want to use std::reverse, as it requires defining an
    //   operator= for Coordinate class (std::swap is used in
    //   std::reverse)

    std::vector<Coordinate> wallCoords;
    wallCoords.push_back( c2 );
    wallCoords.push_back( c1 );

    // Insert corner if necessary
    if ( boundPt1.worldBoundary() != boundPt2.worldBoundary() ) {
      Coordinate cinsert ( getCornerCoordinate( boundPt1.worldBoundary(), boundPt2.worldBoundary()) );
      wallCoords.insert( std::next( wallCoords.begin() ), cinsert );
    }

    std::copy( wallCoords.begin(), 
               wallCoords.end(), 
               std::back_inserter( coordList_ ) );

    return true;

  }


  //============================================
  void CoordinateCollection::printSimpleConfigFile( std::string const& filename ) const {

    std::fstream fs;
    fs.open( filename.data(), std::fstream::out );

    fs << R"(// SimpleConfig geometry file automatically produced for original file: )" << std::endl;
    fs << "//" << std::endl;
    fs << "//   " << inputFile_ << std::endl;
    fs << std::endl;

    // make list
    fs << R"(vector<string> building.)" << volName_ << " = {" << std::endl;
    
    for ( std::size_t i(0);  i < coordList_.size() ; ++i ) {
      fs << "  \"" << coordList_.at(i).inputString() << "\"";
      if ( i != coordList_.size()-1 ) fs << ",";
      fs << std::endl;
      
    }
    
    fs << R"(};)" << std::endl;

    fs << std::endl;

    fs << R"(// Local Variables:)" << std::endl;
    fs << R"(// mode:c++)"         << std::endl;
    fs << R"(// End:)"             << std::endl;
    
    fs.close();

  }

  //============================================
  void CoordinateCollection::printSimpleConfigFileVerbose( std::string const& filename ) const {

    std::fstream fs;
    fs.open( filename.data(), std::fstream::out );

    fs << R"(// SimpleConfig geometry file automatically produced for original file: )" << std::endl;
    fs << "//" << std::endl;
    fs << "//   " << inputFile_ << std::endl;
    fs << std::endl;

    // make list
    fs << R"(vector<double> building.)" << volName_ << ".xPositions = {" << std::endl;
    
    for ( std::size_t i(0);  i < coordList_.size() ; ++i ) {
      fs << "  " << coordList_.at(i).x();
      if ( i != coordList_.size()-1 ) fs << ",";
      fs << std::endl;
      
    }
    
    fs << R"(};)" << std::endl;

    fs << std::endl;

    fs << R"(vector<double> building.)" << volName_ << ".yPositions = {" << std::endl;
    
    for ( std::size_t i(0);  i < coordList_.size() ; ++i ) {
      fs << "  " << coordList_.at(i).y();
      if ( i != coordList_.size()-1 ) fs << ",";
      fs << std::endl;
      
    }
    
    fs << R"(};)" << std::endl;

    fs << R"(// Local Variables:)" << std::endl;
    fs << R"(// mode:c++)"         << std::endl;
    fs << R"(// End:)"             << std::endl;
    
    fs.close();

  }

  //============================================
  bool CoordinateCollection::hasOuterPoints( const CoordinateCollection& ccoll ) {

    unsigned nPoints(0);
    for ( const auto& coord : ccoll.coordinates() ) {
      if ( coord.isOutline() ) ++nPoints;
    }
    return nPoints > 1 ? true : false;

  }

  //============================================
  void CoordinateCollection::checkBoundaryPairForCongruency( const Coordinate& c1,
                                                             const Coordinate& c2 ) {

    int diff1 = c1.worldBoundary()-c2.worldBoundary();
    int diff2 = -diff1;

    if ( diff1 < 0 ) diff1 += 8;
    else diff2 += 8;

    std::ostringstream os;
    os << " Coordinate boundaries of: "
       << c1.worldBoundary()  
       << " and " 
       << c2.worldBoundary() 
       << " are incongruent!";

    if ( diff1 == 4 && diff2 == 4 )
      throw std::runtime_error( os.str() );

  }

  //============================================
  Coordinate CoordinateCollection::getWallCoordinate( const Coordinate& coord, const std::string& label ) const {
    Rep<double> tmp = coord.getCoord();

    switch( coord.worldBoundary() ) {
    case N : tmp.at(1) = worldCorner(NE).at(1); break;
    case E : tmp.at(0) = worldCorner(NE).at(0); break;
    case S : tmp.at(1) = worldCorner(SW).at(1); break;
    case W : tmp.at(0) = worldCorner(SW).at(0); break;
    default : throw std::runtime_error("You should never get here!");
    }
    
    return Coordinate( tmp, label, worldDir::none, true, true );
  }
  
  //============================================
  Coordinate CoordinateCollection::getCornerCoordinate( const worldDir::enum_type type1, 
                                                        const worldDir::enum_type type2 ) const {
    
    static std::vector<worldDir::enum_type> nw {{N,W}};
    static std::vector<worldDir::enum_type> ne {{N,E}};
    static std::vector<worldDir::enum_type> se {{E,S}};
    static std::vector<worldDir::enum_type> sw {{S,W}};
    
    std::vector<worldDir::enum_type> bounds {{ type1, type2 }};
    std::sort( bounds.begin(), bounds.end() );
    
    const Rep<double>* corner;
    
    std::string label;
    if      ( std::equal( bounds.begin(), bounds.end(), nw.begin() ) ) { corner = &worldCorner(NW); label = "NWcorner"; }
    else if ( std::equal( bounds.begin(), bounds.end(), ne.begin() ) ) { corner = &worldCorner(NE); label = "NEcorner"; }
    else if ( std::equal( bounds.begin(), bounds.end(), se.begin() ) ) { corner = &worldCorner(SE); label = "SEcorner"; }
    else if ( std::equal( bounds.begin(), bounds.end(), sw.begin() ) ) { corner = &worldCorner(SW); label = "SWcorner"; }
    
    
    return Coordinate( *corner, label, worldDir::none, true, true );
  }
  
} // end of namespace mu2e


