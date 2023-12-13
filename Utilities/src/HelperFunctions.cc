// C++ includes
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>

// ROOT includes
#include "TChain.h"
#include "TColor.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"

// helper include
#include "Utilities/inc/HelperFunctions.hh"

#include <cmath>

using namespace std;

namespace util {

  //_____________________________
  vector<TString> get_file_list( string dir, string rootfilebase ) {
  
    vector< TString > filelist;

    string cmd_str = "find ./" + dir + "/good/ -name \"" +rootfilebase+".root\"";
    char* cmd = Form(cmd_str.c_str());

    FILE* pipe = popen(cmd,"r");
    if (!pipe) return filelist;
    char buffer[144];
    while(!feof(pipe)) {
      if (fgets(buffer,144,pipe) != nullptr ) {
        // Remove trailing '\n' and "/"
        char* pos;
        if ( (pos=strchr(buffer,'\n')) != nullptr ||
             (pos=strchr(buffer,'/')) != nullptr ) *pos='\0';
        filelist.push_back( (TString)buffer );
      }
    }
    pclose(pipe);

    if ( filelist.empty() ) throw out_of_range( "\n\n No files found corresponding to command: " + cmd_str + "\n" );

    return filelist;

  }

  //_________________________________
  TChain* setupTChains( TString chainName , const vector<TString>& filelist ) {
  
    // Add files onto list
    TChain* chain = new TChain( chainName );
    for ( const TString& filename : filelist ) {
      chain->Add(filename);
    }
    return chain;
  
  }

  //__________________________________
  vector<int> getColorScheme( int ncolors ) {
    vector<int> colors;
  
    double Red[2]    = {0.00,1.00};
    double Green[2]  = {0.00,0.00};
    double Blue[2]   = {1.00,0.00};
    double Length[2] = {0.00,1.00};

    int FI = -1;
    FI = TColor::CreateGradientColorTable(2,Length,Red,Green,Blue,ncolors);
    for (int i = 0 ; i< ncolors; i++ ) colors.push_back( FI+i );

    return colors;

  }

  //_______________________________________________________________________
  void check_argc( int argc, initializer_list<string> arguments ) {

    auto arglistend    = arguments.end(); arglistend--;
    const bool arglist = ( arglistend->find("...") != string::npos ) ? true : false;

    const size_t n = arglist ? arguments.size()-1 : arguments.size();
        
    if ( !arglist && static_cast<size_t>( argc ) == n ) return;
    if (  arglist && static_cast<size_t>( argc ) >= n ) return;


    string errMsg = "\n\n Error in no. of arguments!  Implementation is:\n   ";

    unsigned count(1);
    for ( const auto& arg : arguments ) {
      if      ( count == 1 ) errMsg += arg ;
      else if ( count == n && arglist ) { errMsg += "  <" + arg + "...> "; break; }
      else errMsg += "  <" + arg + "> ";
      count++;
    }
    errMsg += "\n";

    throw invalid_argument( errMsg );

  }

  //_______________________________________________________________________
  string check_argv( string argv, initializer_list<string> argv_options ) {

    string errMsg = "\n\n << " + argv + " >> not an accepted option!  Choices are:\n"   ;

    bool optionPresent( false );
    for ( const auto& option : argv_options ){
      optionPresent |= argv.find( option ) != string::npos;
      errMsg += "   ..." + option + "\n";
    }
    errMsg += "\n";

    if ( !optionPresent) throw invalid_argument( errMsg );
  
    return argv;
  }

  //_______________________________________________________________________
  void check_argv( char** argv, initializer_list<string> argv_options ) {

    const int nargs = sizeof(argv)/sizeof(char*);

    cout << " Nargs: " << nargs << endl;

    for ( int iarg(1) ; iarg<nargs ; iarg++ ) {
      const string arg ( argv[iarg] );
      string errMsg = "\n\n << " + arg + " >> not an accepted option!  Choices are:\n"   ;
      
      bool optionPresent( false );
      for ( const auto& option : argv_options ){
        optionPresent |= arg.find( option ) != string::npos;
        errMsg += "   ..." + option + "\n";
      }
      errMsg += "\n";

      if ( !optionPresent) throw invalid_argument( errMsg );
  
    }
  }

  //_______________________________________________________________________
  string remove_space( string str ) {
    str.erase( remove( str.begin(), str.end(), ' ' ), str.end() );
    return str;
  }

  //_______________________________________________________________________
  bool has_no_character( string str ) {
    str.erase( remove( str.begin(), str.end(), ' ' ), str.end() );
    return str.empty();
  }

  //_______________________________________________________________________
  vector<string> tokenize_str(const string & str,  const string & delims ) {
    // Skip delims at beginning, find start of first token
    string::size_type lastPos = str.find_first_not_of(delims, 0);
    // Find next delimiter @ end of token
    string::size_type pos     = str.find_first_of(delims, lastPos);
    
    // output vector
    vector<string> tokens;
    
    while (string::npos != pos || string::npos != lastPos) {
      // Found a token, add it to the vector.
      string sub = str.substr(lastPos, pos - lastPos);
      if (delims == " " && sub.length() == 0) continue;
      tokens.push_back(sub);
      // Skip delims.  Note the "not_of". this
      // is beginning of token
      lastPos = str.find_first_not_of(delims, pos);
      // Find next delimiter at end of
      // token.
      pos     = str.find_first_of(delims, lastPos);
    }
    
    return tokens;
  }

  //_______________________________________________________________________
  vector<string> initialize_readin_file( const string & file ) {

    ifstream ifile(file.c_str());
    //    string errMsg = "\n\n Could not open: " + file "\n";
    if (!ifile.is_open()) throw invalid_argument( "\n\n Could not open: " + file + "\n" );
  
    vector<string> lines;

    string str;
    while(getline(ifile, str)){
      if (str.find("#") != 0 && str.length()>0) lines.push_back(str);
    }
    ifile.close();

    return lines;
  }

  //_______________________________________________________________________
  map<string,size_t> get_column_headings( const string & firstRow ) {
    vector<string> tmpList    = tokenize_str( firstRow, "|" );
    vector<string> sampleList = tokenize_str( tmpList.back() );
    
    map<string,size_t> columnMap;
    for ( size_t i = 0 ; i < sampleList.size() ; i++ )
      columnMap.insert( make_pair( sampleList.at(i), i ) );

    return columnMap;
  }

}
