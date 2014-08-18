#ifndef util_Coordinate_hh
#define util_Coordinate_hh
//
// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel

//
// Class for converting an (x,y) pair from standard units (formatted
// as a string), to metric units (mm).  The construction of the object looks like:
//
//     Coordinate("xft:xin,yft:yin"), 
//
// where xft, yft are integers expressed within the string, and xin,
// yin are doubles expressed within the string.  Acceptable examples
// include:
//
//     Coordinate("-4:3,5:6") // parsed as x = -4' 3" | y =  5'  6"
//     Coordinate(":3,4")     // parsed as x =  0' 3" | y =  4'  0"
//
// Note the following example:
//
//     Coordinate("2:,-:5")   // parsed as x =  2' 0" | y = -0' +5" 
//
// you probably don't want this, instead you'll have to do something like:
//
//     Coordinate("2:,:-5")   // parsed as x =  2' 0" | y =  0' -5"
//
// Also note that the number preceding the : is converted to an
// integer, whereas the number afterward is cast into a double so that
// floating-point inch values are allowed.  Note that a double value
// for feet does not make sense.  For example
//
//     Coordinate("2.5:2,-3:2") // parsed as x = 2' 2" | y = -3' 2"
//
// No free function currently exists for adding coordinates, but it
// would be straightforward to do if needed.

// C++ includes
#include <array>
#include <iostream>
#include <utility>

namespace util {
  
  class Coordinate {
    
  public:

    typedef std::pair <int,double> FtInchPair;
    template <typename T> using Rep = std::array<T,2>;
    
    // Constructors
    explicit Coordinate( const std::string& input ); 
    
    // Accessors
    bool drawFlag() const { return draw_; }

    const std::string& label()    const { return label_;    }
    const std::string& refLabel() const { return refLabel_; }
    double rot() const { return rotWrtRef_; }

    const Rep<FtInchPair>& getCoordStd() const { return coordStd_; } // Coordinate (wrt ref) in ft. and inches
    const Rep<double>    & getCoordRel() const { return coordRel_; } // Coordinate (wrt ref) in mm
    const Rep<double>    & getCoord()    const { return coord_;    } // Coordinate in mm (absolute position)

    // Metric (mm)
    double get(std::size_t i) const { return coord_.at(i); }
    
    double x() const { return coord_.at(0); }
    double y() const { return coord_.at(1); }

    // Modifiers
    void setAbsX    ( const double x ){ coord_.at(0) = x; };
    void setAbsY    ( const double y ){ coord_.at(1) = y; };
    void setAbsCoord( const Rep<double>& rep ) { coord_ = rep; }
    void setRotation( double rot ){ rotWrtRef_ = rot; }
    void setRefLabel( const std::string& refLabel ) { refLabel_ = refLabel; }
    
    void print() const;

    static FtInchPair makeFtInchPair( const std::string& stringToParse );
    static double     convert2mm    ( const FtInchPair& ftInchPair );

    Rep<FtInchPair> readCoordinatesStd( const std::string& coordStr     );
    Rep<double>     calcRelCoordinates( const Rep<FtInchPair>& coordStd );

  private:

    std::string label_;
    std::string coordStr_;
    std::string refLabel_;
    double      rotWrtRef_;

    bool draw_;

    Rep<FtInchPair> coordStd_;
    Rep<double>     coordRel_;
    Rep<double>     coord_;
    
  };
 
} // end of namespace mu2e

#endif /* util_Coordinate_hh */
