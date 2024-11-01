/**
 * reader.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <reader.h>

namespace cfg
{
  namespace reader
  {
    const bool gmsh_header_parser::is_gmsh_file(const std::filesystem::path &meshfile) const
    {
      std::ifstream istream(meshfile);
      std::string line;
      std::getline(istream, line);
      return (line == "$MeshFormat");
    }

    const std::string gmsh_header_parser::get_header(const std::filesystem::path& meshfile) const
    {
      /*
       * The header contents should be on line 2: discard line 1 and return
       * line 2.
       */
      std::ifstream istream(meshfile);
      std::string line;
      std::getline(istream, line);  // discard
      std::getline(istream, line);
      return line;
    }

    const gmsh_header gmsh_header_parser::parse_header(const std::string& line) const
    {
      auto string2bool = [](const std::string& s) -> bool
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
      auto string2header = [string2bool](const std::string& line, const std::string& version) -> gmsh_header
      {
	std::stringstream ss(line);
	std::string ver, binflag, dsize;
	ss >> ver >> binflag >> dsize;

	if (version != ver)
	{
	  throw std::runtime_error{"GMSH mesh format in file != expected version: " +
				   ver +
				   " vs " +
				   version};
	}

	return gmsh_header{ver, string2bool(binflag), std::stoi(dsize)};
      };
      return string2header(line, version);
    }
    const gmsh_header gmsh_header_parser::parse_header(const std::filesystem::path& meshfile) const
    {
      return parse_header(get_header(meshfile));
    }
  }
}
