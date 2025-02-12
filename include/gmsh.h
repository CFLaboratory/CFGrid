/**
 * gmsh.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_GMSH_H_
#define __CFG_GMSH_H_

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <node_parser.h>
// #include <topo_parser.h>

namespace cfg::mesh
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
      const GmshHeader header               = read_header(mesh_file);
      const auto [stream_mode, reader_mode] = open_mode(header);
      std::ifstream mesh_stream{mesh_file, stream_mode};
      const auto nodes = cfg::parser::read_nodes(mesh_stream, reader_mode, parallel);
      // const auto topo = cfg::parser::read_topo(mesh_stream, reader_mode, nodes, parallel);
      // std::cout << topo << std::endl;
    }

   private:
    /**
     * Convenience function to parse out the header of a GMSH mesh file given the file path.
     *
     * @param meshfile The path to the file.
     * @returns        The GMSH header data structure.
     */
    [[nodiscard]] static GmshHeader read_header(const std::filesystem::path& meshfile);

    [[nodiscard]] static std::tuple<std::ios::openmode, reader::Mode> open_mode(const GmshHeader& header)
    {
      if (header.binary)
      {
        return {std::ios::in | std::ios::binary, reader::Mode::BINARY};
      }
      return {std::ios::in, reader::Mode::ASCII};
    }
  };
}  // namespace cfg::mesh

#endif  // __CFG_GMSH_H_
