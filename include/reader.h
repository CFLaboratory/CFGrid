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
#include <fstream>

#include <node_parser.h>

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
   * Reads a GMSH file.
   */
  class GmshReader
  {
  public:
    /**
     * Constructs a `GmshReader` object.
     *
     * @param mesh_file The filepath to a GMSH file (assumed valid).
     * @param parallel  The parallel environment.
     */
    GmshReader(const std::filesystem::path& mesh_file, const cfg::utils::Parallel& parallel)
    {
      const GmshHeader header = read_header(mesh_file);

      if (header.binary)
      {
	// Binary
	std::ifstream mesh_stream{mesh_file, std::ios::in | std::ios::binary};
	cfg::parser::read_nodes(mesh_stream, cfg::parser::Mode::BINARY, parallel);
      }
      else
      {
	// ASCII
	std::ifstream mesh_stream{mesh_file};
	cfg::parser::read_nodes(mesh_stream, cfg::parser::Mode::ASCII, parallel);
      }
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
