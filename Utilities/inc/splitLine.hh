#ifndef util_splitLine_hh
#define util_splitLine_hh
//
// Split a string, s, into pieces.
//
// Original author Rob Kutschke
//
// Arguments:
//   1 - the string to be split apart
//   2 - the delimiter that marks the pieces
//   3 - the returned pieces
//
// The delimiter characters are not part of any of the returned pieces.
//
// If there are characters between the last delimiter and the end of line,
// those characters for one addition piece.
//
// If the delimiter has a length of zero, then the whole input string is
// returned as the sole piece.
//
// If the input string contains two consecutive delimiters with no
// intervening characters, the code will create a piece that is an
// empty string.
//

#include <string>
#include <vector>

namespace util {

  void splitLine ( std::string const&        s,
                   std::string const&        delimiter,
                   std::vector<std::string>& parts);

} // namespace mu2e

#endif /* util_splitLine_hh */
