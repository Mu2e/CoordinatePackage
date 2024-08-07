// Original author: Kyle Knoepfel

#include "Utilities/inc/Coordinate.hh"
#include "Utilities/inc/splitLine.hh"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <stdexcept>

namespace {

  int sgn( int value ) {
    if  ( value == 0 ) return 1;
    else return value/std::abs(value);
  }

}

namespace util {

  //=========================================================================
  Coordinate::Coordinate( const std::string& inputString )
    : inputString_(inputString)
    , rotWrtRef_(-1000.)
    , draw_(true)
    , isOut_(false)
    , worldBoundary_(worldDir::none)
    , coordStd_( readCoordinatesStd( inputString_ ) )
    , coordRel_( calcRelCoordinates( coordStd_ )    )
  {}

  //=========================================================================
  Coordinate::Coordinate( const Rep<double>& point,
                          const std::string& label,
                          worldDir::enum_type worldBoundary,
                          bool draw,
                          bool isOut )
    : label_(label)
    , refLabel_("*")
    , rotWrtRef_(0.)
    , draw_(draw)
    , isOut_(isOut)
    , worldBoundary_( worldBoundary )
    , coord_( point )
  {}


  //=========================================================================
  Coordinate::Rep<Coordinate::FtInchPair> Coordinate::readCoordinatesStd( const std::string& inputString ) {

    // Assign label, references, and coordinate string assuming form:
    // "B)[N]<A|14>xStd:yStd
    const std::size_t labelDelim     = inputString.find(")");
    const std::size_t wallLeftDelim  = inputString.find("[");
    const std::size_t wallRightDelim = inputString.find("]");
    const std::size_t refLeftDelim   = inputString.find("<");
    const std::size_t refRightDelim  = inputString.find(">");
    const std::size_t rotDelim       = inputString.find("|");

    // Get label
    if ( labelDelim != std::string::npos ) label_ = inputString.substr( 0, labelDelim );
    else throw std::runtime_error( "Label not specified for coordinate: "+inputString );

    // Check if draw flag should be set
    if ( label_.find("//") != std::string::npos ) {
      draw_ = false;
      label_ = label_.substr( label_.find_last_of("/")+1 );
    }

    // Check if coordinate represents point on the outside
    if ( std::islower( label_[0] ) ) isOut_ = true;

    // Make sure there's an ordered pair
    if ( inputString.find(",") == std::string::npos )
      throw std::runtime_error("Label << "+label_+" >> has no ordered pair!  You probably forgot the ',' character." );

    // Get origin reference
    const bool newOrigin = ( refLeftDelim != std::string::npos && refRightDelim != std::string::npos );
    const bool oldOrigin = ( refLeftDelim == std::string::npos && refRightDelim == std::string::npos );
    if ( !newOrigin && !oldOrigin ) throw std::runtime_error(" Missing \"<\" or \">\" for coordinate \""+inputString+"\"");

    // Get wall reference
    const bool yesWallRef  = ( wallLeftDelim != std::string::npos && wallRightDelim != std::string::npos );
    const bool noWallRef   = ( wallLeftDelim == std::string::npos && wallRightDelim == std::string::npos );
    if ( !yesWallRef && !noWallRef ) throw std::runtime_error(" Missing \"[\" or \"]\" for coordinate \""+inputString+"\"");
    if (  yesWallRef && !newOrigin ) throw std::runtime_error(" Must specify reference point \"<something_here>\" when adding wall reference! (for "+inputString+")\n");

    if ( yesWallRef ) {
      worldBoundary_ = worldDir::stringToEnum( inputString.substr( wallLeftDelim+1, wallRightDelim-wallLeftDelim-1 ) );
    }

    // Check for rotation
    const bool newRotation = ( rotDelim != std::string::npos ) && (refRightDelim-rotDelim-1 != 0);

    // Reassign reference origin and rotation as necessary
    if ( newRotation ) rotWrtRef_  = std::atof( inputString.substr(rotDelim+1,refRightDelim-rotDelim-1).c_str() );
    if ( newOrigin   ){
      const std::string bracketString = inputString.substr(refLeftDelim+1,refRightDelim-refLeftDelim-1);
      if      ( !newRotation && refRightDelim-refLeftDelim-1 != 0 && bracketString != "|") refLabel_ = bracketString.substr(0,bracketString.find("|"));
      else if (  newRotation && rotDelim     -refLeftDelim-1 != 0) refLabel_ = inputString.substr(refLeftDelim+1,rotDelim-refLeftDelim-1);
    }

    // Get coordinate string
    coordStr_ =
      refRightDelim != std::string::npos ?
      inputString.substr( refRightDelim+1 ) :
      inputString.substr( labelDelim+1 );

    if ( coordStr_.empty() ) throw std::runtime_error( "No coordinate exists for label << "+label_+" >>!");

    // Parse coordinates
    std::vector<std::string> parts;
    splitLine( coordStr_, ",", parts );

    FtInchPair xStd = makeFtInchPair( parts.at(0) );
    FtInchPair yStd = makeFtInchPair( parts.at(1) );

    return {xStd,yStd};
  }



  //=========================================================================
  Coordinate::Rep<double> Coordinate::calcRelCoordinates( const std::array<FtInchPair,2>& coordStd ) {

    const double x = convert2mm( coordStd.at(0) );
    const double y = convert2mm( coordStd.at(1) );

    return {x,y};
  }

  //=========================================================================
  Coordinate::FtInchPair Coordinate::makeFtInchPair( const std::string& stringToParse ) {
    std::vector<std::string> stringPairing;
    splitLine( stringToParse, ":", stringPairing );

    const int    ft = ( stringPairing.size() > 0 ) ? std::atoi( stringPairing.at(0).c_str() ) : 0;
    const double in = ( stringPairing.size() > 1 ) ? std::atof( stringPairing.at(1).c_str() ) : 0.;

    return {ft,in};
  }

  //=========================================================================
  double Coordinate::convert2mm( const FtInchPair& ftInchPair ) {

    std::string prefix = " Error in Coordinate::convert2mm: ";
    if      ( ftInchPair.first > 0 && ftInchPair.second < 0. ) throw std::runtime_error( prefix+"Cannot have +ft and -in!" );
    else if ( ftInchPair.first < 0 && ftInchPair.second < 0. ) throw std::runtime_error( prefix+"Cannot have -ft and -in!" );

    double inches = ftInchPair.first*12;
    inches += sgn(ftInchPair.first)*ftInchPair.second;
    return inches*25.4;
  }

  //=========================================================================
  void Coordinate::print() const {
    std::cout.setf( std::cout.left );
    std::cout << " Location of coordinate << "
              << std::setw(3) << label_ << " >> : ( " ;
    std::cout.unsetf( std::cout.left );
    std::cout << std::setw(8) << coord_.at(0) << ","
              << std::setw(8) << coord_.at(1) << " ) or [ "
	      << std::setw(4) << coordStd_.at(0).first  << " ft "
              << std::setw(6) << coordStd_.at(0).second << " in  , "
	      << std::setw(4) << coordStd_.at(1).first  << " ft "
              << std::setw(6) << coordStd_.at(1).second << " in ] " ;
    std::cout.setf( std::cout.left );
    std::cout << " wrt label << "
              << std::setw(3) << refLabel_ << " >> at a rotation of : "
              << std::setw(6) << rotWrtRef_
              << " outer: " << isOut_
              << std::endl;
  }

} // end of namespace mu2e
