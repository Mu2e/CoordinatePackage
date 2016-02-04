#ifndef Mu2eUtilities_Table_hh
#define Mu2eUtilities_Table_hh
//
// Free function for loading table Table
//
// $Id$
// $Author$
// $Date$
//
// Original author: Kyle Knoepfel

// C++ includes
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

namespace util {
  
  class Exception : std::exception {
  public :
    Exception( const std::string& notice ) : notice_(notice) { std::cout << " Exception: " << notice << std::endl; }
  private :
    std::string notice_;    
  };

  // global templates for ease of use
  template<const unsigned M> using Value    = std::array<double,M>;
  template<const unsigned N> using TableRow = std::pair<std::string,Value<N-1>>;
  template<const unsigned N> using TableVec = std::vector<TableRow<N>>;

  // definition of class
  template <const unsigned N> class Table {
    
    typedef typename TableVec<N>::const_iterator table_const_iterator;

  public:
    
    // Constructors
    Table(){}
    explicit Table( const TableVec<N>& tablevec ) : rawTable_( tablevec ) {}

    // Accessors
    unsigned            getNrows()         const;
    unsigned            getNcols()         const;
    const TableRow<N>&  getRow( const unsigned key ) const;

    const TableVec<N>& rawTable()          const;
    
    // Helper utilities
    void printRow( unsigned i ) const;
    void printTable()           const;

  private:
    TableVec<N> rawTable_;

    template <const unsigned M> 
    friend Table<M> loadTable( const std::string& tableFile );
    
  };
    


  // end of interface
  //====================================================================================
  // generic implementation below
  // explicit specializations in src/Table.cc file

  template <const unsigned N> unsigned Table<N>::getNrows()               const { return rawTable_.size(); }
  template <const unsigned N> unsigned Table<N>::getNcols()               const { return N; }
  template <const unsigned N> const TableRow<N>& Table<N>::getRow(const unsigned i) const { return rawTable_.at(i);}

  template <const unsigned N> const TableVec<N>& Table<N>::rawTable()     const { return rawTable_; }
  

  template <const unsigned N> void Table<N>::printRow( unsigned i ) const {
    std::cout << rawTable_.at(i).first << " " ;
    for ( auto const & val : rawTable_.at(i).second )
      std::cout << val << " " ;
    std::cout << std::endl;
  }
  
  template <const unsigned N> void Table<N>::printTable() const {
    for ( auto const & it : rawTable_ ) {
      std::cout << it.first << " : " ;
      std::for_each ( it.second.begin(), it.second.end(), [](double val){ std::cout << val << " " ; } );
      std::cout << std::endl;
    }
  }

  //-------------- free function, friend to Table class --------------------------------------------------------
  template <const unsigned N>
  Table<N> loadTable( const std::string& tableFile ) {
      
    std::fstream intable(tableFile.c_str(),std::ios::in);
    if ( !intable.is_open() ) {
      throw Exception("No Tabulated spectrum table file found");
    }
    
    Table<N> tmp_table;

    // Load table
    while ( !intable.eof() ) {
      TableRow<N> tableRow;
      intable >> tableRow.first;              // Get key first
      std::for_each( tableRow.second.begin(), // Now fill the values
                     tableRow.second.end(), 
                     [&](double& d){
                       intable >> d;
                     } );
      if ( !intable.eof() ) {
        tmp_table.rawTable_.emplace_back( tableRow.first, Value<N-1>( tableRow.second ) );
      }
      
    }
    
    return tmp_table;
    
  }
 
} // end of namespace util

#endif 
