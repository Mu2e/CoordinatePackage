#ifndef util_Coordinate_hh
#define util_Coordinate_hh
//
// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel

// See docdb-xxxx for information about how the interface works.

// C++ includes
#include <array>
#include <iostream>
#include <utility>

namespace worldDir {
  enum enum_type { 

    // Instantiation
    none=0, 

    // Cardinal directions (world walls)
    N=2,
    E=4, 
    S=6, 
    W=8,

    // Corners of world
    NW=1,
    NE=3,
    SE=5,
    SW=7
  };
  
  inline enum_type stringToEnum( const std::string& str ) {
    enum_type type (none);
    
    if ( str.length() != 1 ) 
      throw std::runtime_error("Wall option << " +str+ " >> is not supported!");

    switch( str.at(0) ) {
    case 'N' : type = N; break;
    case 'E' : type = E; break;
    case 'S' : type = S; break;
    case 'W' : type = W; break;
    default: 
      throw std::runtime_error("Wall option << " +str+ " >> is not supported!");
    }
    
    return type;
  }

}

namespace util {
  
  class Coordinate {
    
  public:

    typedef std::pair <int,double> FtInchPair;
    template <typename T> using Rep = std::array<T,2>;
    
    // Constructors
    explicit Coordinate( const std::string& input ); 

    explicit Coordinate( const Rep<double>& point,
                         const std::string& label,
                         worldDir::enum_type worldBoundary,
                         bool draw,
                         bool isOut );
    
    // Accessors
    const std::string& inputString() const { return inputString_; }

    bool drawFlag()  const { return draw_;  }
    bool isOutline() const { return isOut_; }
    worldDir::enum_type worldBoundary() const { return worldBoundary_; }

    const std::string& label()    const { return label_;     }
    const std::string& refLabel() const { return refLabel_;  }
    double rot()                  const { return rotWrtRef_; }

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

    std::string inputString_;

    std::string label_;
    std::string coordStr_;
    std::string refLabel_;
    double      rotWrtRef_;

    bool draw_;
    bool isOut_;
    worldDir::enum_type worldBoundary_;

    Rep<FtInchPair> coordStd_;
    Rep<double>     coordRel_;
    Rep<double>     coord_;
    
  };
 
} // end of namespace mu2e

#endif /* util_Coordinate_hh */
