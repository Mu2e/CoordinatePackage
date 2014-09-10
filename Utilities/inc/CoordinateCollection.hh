#ifndef util_CoordinateCollection_hh
#define util_CoordinateCollection_hh
//
// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel


// Utilities includes
#include "Utilities/inc/Coordinate.hh"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>

namespace util {
  
  class CoordinateCollection {
    
  public:
    
    template <typename T> using Rep = Coordinate::Rep<T>;

    // Constructors
    explicit CoordinateCollection( const std::string& inputCollection,
                                   const std::map<worldDir::enum_type,Coordinate::Rep<double>>& worldCorners); 
    
    const std::string&             volName()     const { return volName_;   }

    const std::vector<Coordinate>& coordinates() const { return coordList_; }
    const Rep<double>&             height()      const { return height_;    }

    const Rep<double>& worldCorner(worldDir::enum_type i) const { 
      return worldCorners_.find(i)->second; 
    }

    void printSimpleConfigFile( std::string const & dir, const bool outline = false ) const;

    void setName( const std::string& name ) { volName_ = name; }
    bool addWorldBoundaries(const bool verbose = false );
    
    static bool hasOuterPoints( const CoordinateCollection& ccoll );

    // These static members indicate the offset of the volume origin
    // wrt the mu2e origin.
    static constexpr double Xoffset = -3581.40;
    static constexpr double Yoffset =   947.40;

  private:
    
    std::string inputFile_;
    std::map<worldDir::enum_type,Rep<double>> worldCorners_;

    std::string volName_;

    std::set<std::string> keyList_;
    std::vector<Coordinate>  coordList_;
    std::vector<Coordinate>  boundaryList_;
    Rep<double> height_;

    std::string assignVolName( const std::string& inputString );
    Rep<double> assignHeight ( const std::string& inputString );
    void check_and_push_back( Coordinate& coordStr );

    Coordinate getReferenceCoordinate( const std::string& refLabel ) const;

    Coordinate getWallCoordinate     ( const Coordinate& c1, const std::string& label ) const;
    Coordinate getCornerCoordinate   ( const worldDir::enum_type type1, const worldDir::enum_type type2 ) const;
    
    static void checkBoundaryPairForCongruency( const Coordinate& c1, const Coordinate& c2 );

  };
 
} // end of namespace util

#endif /* util_CoordinateCollection_hh */
