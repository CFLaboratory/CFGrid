/**
 * reader.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fstream>
#include <sstream>

#include <reader.h>

namespace cfg
{
  namespace reader
  {
    [[nodiscard]] const GmshHeader GmshHeaderParser::parse_header(const std::string& line) const
    {
      auto string2bool = [](const std::string& s) -> const bool
      {
        /* Only strings "0", "1" are treated as valid bools */
        if (s == "0")
        {
          return false;
        }
        else if (s == "1")
        {
          return true;
        }
        else
        {
          throw std::runtime_error{"Can't convert string to bool: " + s};
        }
      };

      /* Deconstruct the header string into components. */
      auto string2header = [string2bool](const std::string& line, const std::string& version) -> const GmshHeader
      {
        std::stringstream ss(line);
        std::string ver, binflag, dsize;
        ss >> ver >> binflag >> dsize;

        if (version != ver)
        {
          throw std::runtime_error{"GMSH mesh format in file != expected version: " + ver + " vs " + version};
        }

        return GmshHeader{ver, string2bool(binflag), std::stoi(dsize)};
      };
      return string2header(line, version);
    }

    [[nodiscard]] const GmshHeader GmshReader::read_header(const std::filesystem::path& meshfile) const
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
  }  // namespace reader
}  // namespace cfg
