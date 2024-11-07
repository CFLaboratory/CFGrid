/**
 * reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_READER_H_
#define __CFG_READER_H_

#include <filesystem>
#include <string>

namespace cfg
{
  namespace reader
  {
    /**
     * A structure containing the header contents of a GMSH mesh file.
     */
    struct GmshHeader
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
      GmshHeader(const std::string& version, const bool& binary, const int& dsize) :
          version(version), binary(binary), dsize(dsize){};
    };

    /**
     * A versioned parser for GMSH file headers.
     */
    class GmshHeaderParser
    {
     public:
      /**
       * Constructor for the `GmshHeaderParser`, specifies the GMSH format version the parser is
       * intended for.
       */
      GmshHeaderParser(const std::string& version) : version(version){};

      /**
       * Parses the header string of a GMSH mesh file.
       *
       * @param line The header string.
       * @returns    The GMSH header data structure.
       */
      [[nodiscard]] const GmshHeader parse_header(const std::string& line) const;
     private:
      std::string version;  // What version is this parser for?
    };

    /**
     */
    class GmshReader
    {
      GmshReader(const std::filesystem::path &meshfile)
      {
	const GmshHeader header = read_header(meshfile);
      }
      
    private:
      /**
       * Convenience function to parse out the header of a GMSH mesh file given the file path.
       *
       * @param meshfile The path to the file.
       * @returns        The GMSH header data structure.
       */
      [[nodiscard]] const GmshHeader read_header(const std::filesystem::path& meshfile) const;
    };
  }  // namespace reader
}  // namespace cfg

#endif // __CFG_READER_H_
