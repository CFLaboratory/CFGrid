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
    enum class MeshFormat {GMSH_ASCII, GMSH_BIN, UNKNOWN};
    
    class unknown_format : public std::runtime_error
    {
    public:
      unknown_format(const std::string &msg) : std::runtime_error{msg} {};
    };

    class reader
    {
    public:
      explicit reader() = default;
      explicit reader(const std::string &meshfile) : meshfile(meshfile)
      {
	// Confirm existence of meshfile, allowing it to throw on failure.
	std::error_code ec;
	if(!std::filesystem::exists(this->meshfile, ec))
	{
	  throw std::filesystem::filesystem_error{"Could not find mesh file " + meshfile, ec};
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

      const MeshFormat format()
      {
	return this->fmt;
      }
    private:
      std::filesystem::path meshfile;
      MeshFormat fmt;
    };
  }  // namespace reader
}  // namespace cfg
  
#endif // __CFG_READER_H_
