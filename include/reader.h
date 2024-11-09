/**
 * reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_READER_H_
#define __CFG_READER_H_

#include <filesystem>
#include <string>
#include <utility>

namespace cfg::reader
{
  /**
   * A structure containing the header contents of a GMSH mesh file.
   */
  struct GmshHeader
  {
    std::string version;  ///< The GMSH mesh file format version.
    bool binary;          ///< Flag indicating whether the mesh file is in binary or ASCII format.
    size_t dsize;         ///< Value indicating the number of bytes used for floating point numbers.

    /**
     * Constructor for the `gmsh_header` object.
     *
     * @param version A version string, for example "4.1".
     * @param binary  A flag indicating whether the file is in binary or ASCII format.
     * @param dsize   The size in bytes of floating point numbers in the file.
     */
    GmshHeader(std::string version, const bool binary, const size_t dsize)
        : version(std::move(version)), binary(binary), dsize(dsize){};
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
    GmshHeaderParser(std::string version) : version(std::move(version)){};

    /**
     * Parses the header string of a GMSH mesh file.
     *
     * @param line The header string.
     * @returns    The GMSH header data structure.
     */
    [[nodiscard]] GmshHeader parse_header(const std::string& line) const;

   private:
    std::string version;  // What version is this parser for?
  };

  /**
   */
  class GmshReader
  {
    GmshReader(const std::filesystem::path& meshfile)
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
    [[nodiscard]] static GmshHeader read_header(const std::filesystem::path& meshfile);
  };
}  // namespace cfg::reader

#endif  // __CFG_READER_H_
