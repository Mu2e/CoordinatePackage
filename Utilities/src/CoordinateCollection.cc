// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel

#include "Utilities/inc/CoordinateCollection.hh"
#include "Utilities/inc/Table.hh"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace util {
  
  //=========================================================================
  CoordinateCollection::CoordinateCollection( const std::string& inputFile ) {
    Table<1> inputTable = loadTable<1>( inputFile );
    unsigned counter(0);
    for( const auto& row : inputTable.rawTable() ) {
      if ( counter == 0 ) height_ = assignHeight( row.first );
      else {
        Coordinate coordStr ( row.first );
        check_and_push_back( coordStr );
      }
      ++counter;
    }
  }
  
  //=========================================================================
  Coordinate::Rep<double> CoordinateCollection::assignHeight( const std::string& inputString ) {
    if ( inputString.find("Height)") == std::string::npos ) 
      throw std::runtime_error("Height of collection not specified!  First label must be:\n \"Height)...,...\"");

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


} // end of namespace mu2e


