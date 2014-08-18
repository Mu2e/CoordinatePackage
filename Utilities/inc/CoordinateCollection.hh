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
#include <set>
#include <string>
#include <vector>

namespace util {
  
  class CoordinateCollection {
    
  public:
    
    template <typename T> using Rep = Coordinate::Rep<T>;

    // Constructors
    explicit CoordinateCollection( const std::string& inputCollection ); 
    
    const std::vector<Coordinate>& coordinates() const { return coordList_; }
    const Rep<double>&             height()      const { return height_;    }

  private:

    
    std::set<std::string> keyList_;
    std::vector<Coordinate>  coordList_;
    Rep<double> height_;

    Rep<double> assignHeight( const std::string& inputString );
    void check_and_push_back( Coordinate& coordStr );

    Coordinate getReferenceCoordinate( const std::string& refLabel ) const;

  };
 
} // end of namespace util

#endif /* util_CoordinateCollection_hh */
