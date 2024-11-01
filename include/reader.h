/**
 * reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_READER_H_
#define __CFG_READER_H_

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <tuple>

namespace cfg
{
  namespace reader
  {
    /**
     * Enum identifying the supported mesh formats that `CFGrid` reads.
     *
     * @note The "supported" `UNKNOWN` format is used to identify when `CFGrid` cannot identify the
     *       mesh format and would generally result in an error being raised.
     */
    enum class MeshFormat {GMSH_ASCII, GMSH_BIN, UNKNOWN};
    
    /**
     * A custom exception class that is raised by `CFGrid` when given an unknown mesh format.
     */
    class unknown_format : public std::runtime_error
    {
    public:
      unknown_format(const std::string &msg) : std::runtime_error{msg} {};
    };

    /**
     * The reader class.
     */
    class reader
    {
    public:
      /**
       * Default constructor for the reader class.
       *
       * @returns An uninitialised `reader` object.
       */
      explicit reader() = default;

      /**
       * The constructor for the reader class.
       *
       * Given a path string to a mesh file the constructor performs validation that: a) the path
       * exists, and b) the mesh file pointed to by the path is a known format according to the
       * `MeshFormat` enum.
       *
       * @param[in] meshfile The path string to the mesh file to be read.
       * @returns            An initialised `reader` object.
       */
      explicit reader(const std::filesystem::path meshfile) : meshfile(meshfile)
      {
	// Confirm existence of meshfile, allowing it to throw on failure.
	std::error_code ec;
	if(!std::filesystem::exists(meshfile, ec))
	{
	  throw std::filesystem::filesystem_error{"Could not find mesh file " + meshfile.string(), ec};
	}

	fmt = get_format(this->meshfile);

	if (MeshFormat::UNKNOWN == fmt)
	{
	  throw cfg::reader::unknown_format{"Reading " + this->meshfile.string() +
					    " : only GMSH format 4.1 is currently supported"};
	}
      }

      /**
       * Utility function to determine the format of a mesh file.
       *
       * @param meshfile Path that is (potentially) pointing to a mesh file.
       * @returns The `MeshFormat` enum value representing the format of the mesh being read. 
       */
      [[nodiscard]] const MeshFormat get_format(const std::filesystem::path &meshfile)
      {
	/*
	 * Currently we only support GMSH files, if the path is a directory then
	 * we can immediately discard it
	 */
        if (!std::filesystem::is_regular_file(meshfile))
        {
          return MeshFormat::UNKNOWN;
        }

	/* Are we reading a GMSH file? */
	auto is_gmsh_file = [](const std::filesystem::path &meshfile) -> bool
	{
	  std::ifstream istream(meshfile);
	  std::string line;
	  std::getline(istream, line);
	  return (line == "$MeshFormat");
	};
	if (!is_gmsh_file(meshfile))
	{
          return MeshFormat::UNKNOWN;
	}

        // GMSH file

	auto get_gmsh_header = [](const std::filesystem::path &meshfile) -> std::string
	{
	  /*
	   * The header contents should be on line 2: discard line 1 and return line 2.
	   */
	  std::ifstream istream(meshfile);
	  std::string line;
	  std::getline(istream, line); // discard
	  std::getline(istream, line);
	  return line;
	};

        auto parse_gmsh_header = [](const std::string &line) -> std::tuple<std::string,
									  bool,
									  int>
        {
          auto string2bool = [](const std::string &s) -> bool
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

	  auto string2header = [string2bool](const std::string &line) -> std::tuple<std::string,
	                                                                            bool,
										    int>
	  {
	    /* Deconstruct the header string into typed components. */
	    std::stringstream ss(line);
	    std::string ver, binflag, dsize;
	    ss >> ver >> binflag >> dsize;
	    return std::tuple<std::string, bool, int>{ver, string2bool(binflag), std::stoi(dsize)};
	  };
	  return string2header(line);
        };

        auto validate_gmsh_header = [](const std::tuple<std::string, bool, int> header) ->
	  MeshFormat
        {
	  auto [ver, binflag, dsize] = header;

          if (ver != "4.1")
          {
            return MeshFormat::UNKNOWN;
          }

          if (binflag)
          {
            return MeshFormat::GMSH_BIN;
          }
          else
          {
            return MeshFormat::GMSH_ASCII;
          }
        };
	
	const auto header_line = get_gmsh_header(meshfile);
	const auto header = parse_gmsh_header(header_line);
	return validate_gmsh_header(header);
      }

      /**
       * Queries the mesh format being read.
       *
       * @returns The `MeshFormat` enum value representing the format of the mesh being read. 
       */
      const MeshFormat format()
      {
	return this->fmt;
      }
    private:
      std::filesystem::path meshfile; // Path to the mesh to read
      MeshFormat fmt;                 // Format of the mesh to read
    };
  }  // namespace reader
}  // namespace cfg
  
#endif // __CFG_READER_H_
