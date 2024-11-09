/**
 * reader.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fstream>
#include <sstream>

#include <reader.h>

namespace cfg::reader
{
  [[nodiscard]] GmshHeader GmshHeaderParser::parse_header(const std::string& line) const
  {
    auto string2bool = [](const std::string& str) -> bool
    {
      /* Only strings "0", "1" are treated as valid bools */
      if ((str != "0") && (str != "1"))
      {
        throw std::runtime_error{"Can't convert string to bool: " + str};
      }

      return (str == "1");
    };

    /* Deconstruct the header string into components. */
    auto string2header = [string2bool](const std::string& line, const std::string& version) -> GmshHeader
    {
      std::stringstream sstream(line);
      std::string ver;
      std::string binflag;
      std::string dsize;
      sstream >> ver >> binflag >> dsize;

      if (version != ver)
      {
        throw std::runtime_error{"GMSH mesh format in file != expected version: " + ver + " vs " + version};
      }

      return GmshHeader{ver, string2bool(binflag), static_cast<size_t>(std::stoi(dsize))};
    };
    return string2header(line, version);
  }

  [[nodiscard]] GmshHeader GmshReader::read_header(const std::filesystem::path& meshfile) 
  {
    /*
     * The header contents should be on line 2: discard line 1 and return
     * line 2.
     */
    std::ifstream istream(meshfile);
    std::string line;
    std::getline(istream, line);  // discard
    std::getline(istream, line);

    const GmshHeaderParser parser("4.1");
    return parser.parse_header(line);
  }
}  // namespace cfg::reader
