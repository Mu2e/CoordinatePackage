#ifndef CONFIG
#define CONFIG

// C++ includes
#include <fstream>
#include <string>
#include <vector>

namespace util {

  struct Config {
    std::vector<std::string> bldgFiles;
    std::vector<std::string> dirtFiles;

    std::vector<std::string> bldgPrefixes;
    std::vector<std::string> dirtPrefixes;
  };
}

#endif /*CONFIG*/
