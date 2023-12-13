#ifndef HELPERFUNCTIONS
#define HELPERFUNCTIONS

// C++ includes
#include <map>
#include <vector>
#include <string>

// ROOT includes
#include "TChain.h"
#include "TString.h"

namespace util {

  // declare functions
  std::vector<TString> get_file_list( std::string dir, std::string rootfilebase = "*" );
  TChain* setupTChains( TString chainName, const std::vector<TString>& filelist );
  std::vector<int> getColorScheme( int ncolors );

  void check_argc( int argc, std::initializer_list<std::string> arguments );
  std::string check_argv( std::string argv, std::initializer_list<std::string> argv_options );
  void check_argv( char** argv, std::initializer_list<std::string> arguments );

  std::string remove_space( std::string str );
  bool has_no_character( std::string str );
  std::vector<std::string>          tokenize_str          ( const std::string& str, const std::string& delims = ", \t");
  std::vector<std::string>          initialize_readin_file( const std::string& filename );
  std::map<std::string,std::size_t> get_column_headings   ( const std::string& firstRow );

}

#endif /* HELPERFUNCTIONS */
