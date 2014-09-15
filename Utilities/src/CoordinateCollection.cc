// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel

#include "Utilities/inc/Config.hh"
#include "Utilities/inc/CoordinateCollection.hh"
#include "Utilities/inc/Table.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "boost/tokenizer.hpp"

using namespace worldDir;

namespace {

  bool hasDirt( std::string str ) {
    return str.find("dirt.") != std::string::npos;
  }

  void replacementStream( std::ostringstream& os, 
                          const std::string& varprefix, 
                          const std::vector<std::size_t>& v ) {
    if ( !v.empty() ) {
      os << "vector<int>    " << varprefix << "replace   = { ";
      std::size_t counter(0);
      for ( std::size_t index : v ) {

        if ( counter != v.size()-1 ) {
          os << index << ", ";
        }
        else {
          os << index << " }; " ;
          os << std::endl;
        }

        ++counter;
      }
    }
  }

}


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
      throw std::runtime_error("\nVolume name not specified in file: "+inputFile_+"\nFirst label must be:\n \"VolName)..a..string..\"");

    const std::size_t delimPos = inputString.find(")");
    return inputString.substr(delimPos+1, inputString.length() );
  }

  //=========================================================================
  Coordinate::Rep<double> CoordinateCollection::assignHeight( const std::string& inputString ) {
    if ( inputString.find("Height)") == std::string::npos ) 
      throw std::runtime_error("\nHeight of solid not specified in file: "+inputFile_+"\nSecond label must be:\n \"Height)...,...\"");

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
    if ( coord.worldBoundary() != worldDir::none ) boundaryList_.push_back( coord );        
    
    coordList_.push_back( coord );

  }

  //============================================
  Coordinate CoordinateCollection::getReferenceCoordinate( const std::string& refLabel ) const {
    
    auto match  = std::find_if( std::begin( coordList_ )
                                ,std::end( coordList_)
                                ,[&](const Coordinate& cs){ return !cs.label().compare( refLabel ); } );
    
    if ( match == std::end( coordList_ ) )
      throw std::runtime_error("Reference coordinate << "+refLabel+" >> notFound!");
    
    return *match;    
  }

  //============================================
  bool CoordinateCollection::addWorldBoundaries( const bool verbose ) {
    
    if ( boundaryList_.size() < 2 ) {
      if ( verbose ) std::cout << " Not enough boundary points present " << std::endl;
      return false;
    }

    // Check for congruency of points
    for ( auto point = std::next( boundaryList_.cbegin() ) ; point != boundaryList_.cend() ; ++point ) {
      checkBoundaryPairForCongruency( *point, *std::prev( point ) );
    }

    // Add wall coordinates
    std::vector<Coordinate> wallCoords;
    for ( const auto& point : boundaryList_ ) {
      std::ostringstream os; os << point.label() << "_to_" << worldDir::enumToString( point.worldBoundary() );
      wallCoords.push_back( getWallCoordinate( point, os.str() ) );
    }
    
    // Reverse wall coordinate list to make consistent with handedness
    // of polygon definition
    std::reverse( wallCoords.begin(), wallCoords.end() );

    // Insert corners if necessary 
    // -- need to use reverse iterator since boundaryList is in
    //    opposite handedness of dirt polygon
    for ( auto point = std::next( boundaryList_.crbegin() ) ; point != boundaryList_.crend() ; ++point ) {

      const worldDir::enum_type b1 = std::prev( point )->worldBoundary();
      const worldDir::enum_type b2 = point->worldBoundary();

      if ( b1 != b2 ) {
        const auto& insertPoint = std::find_if( wallCoords.begin()
                                                ,wallCoords.end()
                                                ,[&](Coordinate c){ 
                                                  return c.worldBoundary() == point->worldBoundary(); 
                                                } );

        wallCoords.insert( insertPoint, getCornerCoordinate( b1,b2 ) );
      }

    }

    std::copy( wallCoords.begin(), 
               wallCoords.end(), 
               std::back_inserter( coordList_ ) );

    return true;

  }


  //============================================
  void CoordinateCollection::printSimpleConfigFile( Config& config, const std::string& dir, const bool outline ) const {

    // Tokenize
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    boost::char_separator<char> sep(".");
    tokenizer tokens(volName_, sep);

    std::string name;
    int tokenCounter(0);
    for ( auto& token : tokens ) {
      // Capitalize first letter
      std::string tmp (token); // need to form a temporary since token iterators are const.
      if ( tokenCounter ) tmp[0] = std::toupper( tmp[0] ); 
      ++tokenCounter;
      name += tmp;
    }

    const std::string filename  = dir+name+".txt";
    const std::string includefilename = "Mu2eG4/geom/bldg/"+name+".txt";
    const std::string varprefix = hasDirt( volName_ ) ? volName_ : "building."+volName_;
    const std::string material  = hasDirt( volName_ ) ? "MBOverburden" : "CONCRETE_MARS";

    // Fill master config
    auto& fileList   = hasDirt( volName_ ) ? config.dirtFiles    : config.bldgFiles;
    auto& prefixList = hasDirt( volName_ ) ? config.dirtPrefixes : config.bldgPrefixes;

    fileList  .push_back( "#include \""+includefilename+"\"" );
    prefixList.push_back( varprefix );

    std::fstream fs;
    fs.open( filename.data(), std::fstream::out );

    const double yHalfThickness = 0.5*(height_.at(1)-height_.at(0));
    const double yOffset        = height_.at(0) + yHalfThickness;

    fs << R"(// SimpleConfig geometry file automatically produced for original file: )" << std::endl;
    fs << "//" << std::endl;
    fs << "//   " << inputFile_ << std::endl;
    fs << std::endl;
    fs << "string " << varprefix << ".name     = \"" << name << "\";" << std::endl;
    fs << std::endl;
    fs << "string " << varprefix << ".material = \"" << material << "\";" << std::endl;
    fs << std::endl;
    fs << "double " << varprefix << ".offsetFromMu2eOrigin.x   = " << Xoffset << ";" << std::endl;
    fs << "double " << varprefix << ".offsetFromFloorSurface.y = " << yOffset << ";" << std::endl;
    fs << "double " << varprefix << ".offsetFromMu2eOrigin.z   = " << Zoffset << ";" << std::endl;
    fs << std::endl;
    fs << "double " << varprefix << ".yHalfThickness           = " << yHalfThickness << ";" << std::endl;
    fs << std::endl;

    std::vector<std::size_t> xReplaceV;
    std::vector<std::size_t> yReplaceV;

    std::ostringstream xstr;
    std::ostringstream ystr;
    // make list
    xstr << R"(vector<double> )" << varprefix << ".xPositions = {" << std::endl;
    ystr << R"(vector<double> )" << varprefix << ".yPositions = {" << std::endl;
    
    std::size_t indexCounter(0);
    for ( std::size_t i(0) ;  i < coordList_.size() ; ++i ) {
      const auto& coord = coordList_.at(i);

      if ( i == 0 || !coord.drawFlag()   ) continue;
      if ( outline && !coord.isOutline() ) continue;

      xstr << "  " << coord.x();
      ystr << "  " << coord.y();
      if ( i != coordList_.size()-1 ) { xstr << ","; ystr << ","; }
      xstr << R"(   // )" << coord.label() << std::endl;
      ystr << R"(   // )" << coord.label() << std::endl;

      // Determine replacement for actual Mu2e world boundaries
      if ( coord.label().find("corner") != std::string::npos ) {
        xReplaceV.push_back( indexCounter );
        yReplaceV.push_back( indexCounter );
      }
      if ( coord.label().find("_to_2")  != std::string::npos ||
           coord.label().find("_to_6")  != std::string::npos ) {
        yReplaceV.push_back( indexCounter );
      }
      if ( coord.label().find("_to_4")  != std::string::npos ||
           coord.label().find("_to_8")  != std::string::npos ) {
        xReplaceV.push_back( indexCounter );
      }

      ++indexCounter;
    }
    xstr <<  R"(};)" << std::endl;
    ystr <<  R"(};)" << std::endl;
    
    // Having trouble getting the move semantics to work for
    // ostringstream (perhaps it's still a bug in the compiler?), so
    // passing the oss by reference.

    std::ostringstream xReplaceOS; replacementStream( xReplaceOS, varprefix+".x", xReplaceV );
    std::ostringstream yReplaceOS; replacementStream( yReplaceOS, varprefix+".y", yReplaceV );

    fs << xReplaceOS.str();
    fs << xstr.str() ;

    fs << std::endl;
    
    fs << yReplaceOS.str();
    fs << ystr.str() ;

    fs << std::endl;
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
    
    return Coordinate( tmp, label, coord.worldBoundary(), true, true );
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


