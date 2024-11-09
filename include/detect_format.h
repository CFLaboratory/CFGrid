/**
 * detect_format.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_DETECT_FORMAT_H_
#define __CFG_DETECT_FORMAT_H_

#include <filesystem>
#include <fstream>
#include <string>

namespace cfg::reader
{
  /**
   * Enum identifying the supported mesh formats that `CFGrid` reads.
   */
  enum class MeshFormat
  {
    GMSH
  };

  /**
   * A custom exception class that is raised by `CFGrid` when given an unknown mesh format.
   */
  class unknown_format : public std::runtime_error
  {
   public:
    unknown_format(const std::string& msg) : std::runtime_error{msg} {};
  };

  /**
   * A class that determines if a given mesh file is a GMSH file.
   */
  class GmshDetector
  {
   public:
    [[nodiscard]] static bool is_gmsh_file(const std::filesystem::path& meshfile)
    {
      std::ifstream istream(meshfile);

      std::string first_line;
      std::getline(istream, first_line);
      return is_gmsh_file(first_line);
    }

    [[nodiscard]] static bool is_gmsh_file(const std::string& first_line)
    {
      // This could be improved by stripping spaces, but should do for now.
      return (first_line == "$MeshFormat") || (first_line == " $MeshFormat") || (first_line == "$MeshFormat ") ||
             (first_line == " $MeshFormat ");
    }
  };

  /**
   * A class that determines the format of a given mesh file.
   */
  class FormatDetector
  {
   public:
    /**
     * Determine the format of a mesh file.
     *
     * @param meshfile Path that is (potentially) pointing to a mesh file.
     * @returns The `MeshFormat` enum value representing the format of the mesh being read.
     */
    [[nodiscard]] static MeshFormat get_format(const std::filesystem::path& meshfile)
    {
      check_mesh_exists(meshfile);

      /*
       * Currently we only support GMSH files, if the path is a directory then
       * we can immediately discard it
       */
      if (!std::filesystem::is_regular_file(meshfile))
      {
        throw unknown_format{"Meshfile: " + meshfile.string() + " is a directory"};
      }

      /* Are we reading a GMSH file? */
      if (cfg::reader::GmshDetector::is_gmsh_file(meshfile))
      {
        return MeshFormat::GMSH;
      }

      throw unknown_format{"Could not determine format of " + meshfile.string()};
    }

   private:
    /**
     * Check that the mesh file does, in fact, exist.
     *
     * Throws a `std::filesystem::filesystem_error` if not.
     */
    static void check_mesh_exists(const std::filesystem::path& meshfile)
    {
      std::error_code code;
      if (!std::filesystem::exists(meshfile, code))
      {
        throw std::filesystem::filesystem_error{"Could not find mesh file " + meshfile.string(), code};
      }
    }
  };
}  // namespace cfg::reader

#endif  // __CFG_DETECT_FORMAT_H_
