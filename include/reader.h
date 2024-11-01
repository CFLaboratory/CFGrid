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

	fmt = MeshFormat::UNKNOWN;
	
	// Attempt to determine the format of the file
	if (std::filesystem::is_regular_file(this->meshfile))
	{
	  // Attempt to read GMSH header
	  {
	    std::ifstream istream(this->meshfile);
	    std::string line;
	    std::getline(istream, line);
	    if (line == "$MeshFormat")
	    {
	      // GMSH file
	      std::getline(istream, line);
	      std::stringstream ss(line);
	      std::string ver, binflag, dsize;
	      ss >> ver >> binflag >> dsize;

	      if (ver == "4.1")
	      {
		if (binflag == "0")
		{
		  fmt = MeshFormat::GMSH_ASCII;
		}
		else if (binflag == "1")
		{
		  fmt = MeshFormat::GMSH_BIN;
		}
		else
		{
		  throw std::runtime_error{"Attempting to read malformed GMSH file"};
		}
	      }
	    }
	  }

	  if (MeshFormat::UNKNOWN == fmt)
	  {
	    throw cfg::reader::unknown_format{"Reading " + this->meshfile.string() +
					      " : only GMSH format 4.1 is currently supported"};
	  }
	}
	else
	{
	  throw cfg::reader::unknown_format{this->meshfile.string() + " is an unrecognised format"};
	}
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
