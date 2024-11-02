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
     * A structure containing the header contents of a GMSH mesh file.
     */
    struct gmsh_header
    {
     public:
      std::string version; ///< The GMSH mesh file format version.
      bool binary;         ///< Flag indicating whether the mesh file is in binary or ASCII format.
      int dsize;           ///< Value indicating the number of bytes used for floating point numbers.

      /**
       * Constructor for the `gmsh_header` object.
       *
       * @param version A version string, for example "4.1".
       * @param binary  A flag indicating whether the file is in binary or ASCII format.
       * @param dsize   The size in bytes of floating point numbers in the file.
       */
      gmsh_header(const std::string& version, const bool& binary, const int& dsize) :
          version(version), binary(binary), dsize(dsize){};
    };

    /**
     * A versioned parser for GMSH file headers.
     */
    class gmsh_header_parser
    {
     public:
      /**
       * Constructor for the `gmsh_header_parser`, specifies the GMSH format version the parser is
       * intended for.
       */
      gmsh_header_parser(const std::string& version) : version(version){};

      /**
       * Determines if a file is a GMSH mesh file.
       *
       * @param meshfile The path to the file.
       * @returns        Flag indicating whether `meshfile` is a GMSH mesh file or not.
       */
      [[nodiscard]] const bool is_gmsh_file(const std::filesystem::path& meshfile) const;

      /**
       * Extracts the header from a GMSH mesh file.
       *
       * @param meshfile The path to the file.
       * @returns        The header string.
       */
      [[nodiscard]] const std::string get_header(const std::filesystem::path& meshfile) const;

      /**
       * Parses the header string of a GMSH mesh file.
       *
       * @param line The header string.
       * @returns    The GMSH header data structure.
       */
      [[nodiscard]] const gmsh_header parse_header(const std::string& line) const;

      /**
       * Convenience function to parse out the header of a GMSH mesh file given the file path.
       *
       * @param meshfile The path to the file.
       * @returns        The GMSH header data structure.
       */
      [[nodiscard]] const gmsh_header parse_header(const std::filesystem::path& meshfile) const;
     private:
      std::string version;  // What version is this parser for?
    };

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
	const gmsh_header_parser parser("4.1");
	if (!parser.is_gmsh_file(meshfile))
	{
          return MeshFormat::UNKNOWN;
	}

        // GMSH file

        auto get_gmsh_format = [](const gmsh_header &header) -> const MeshFormat
        {
          if (header.binary)
          {
            return MeshFormat::GMSH_BIN;
          }
          else
          {
            return MeshFormat::GMSH_ASCII;
          }
        };
	
	const auto header = parser.parse_header(meshfile);
	return get_gmsh_format(header);
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
