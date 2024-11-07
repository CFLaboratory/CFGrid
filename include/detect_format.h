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

namespace cfg
{
  namespace reader
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
      [[nodiscard]] bool is_gmsh_file(const std::filesystem::path& meshfile) const
      {
        std::ifstream istream(meshfile);

        std::string first_line;
        std::getline(istream, first_line);
        return is_gmsh_file(first_line);
      }

      [[nodiscard]] bool is_gmsh_file(const std::string& first_line) const
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
      [[nodiscard]] const MeshFormat get_format(const std::filesystem::path& meshfile) const
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
        const GmshDetector gmsh_detector;
        if (gmsh_detector.is_gmsh_file(meshfile))
        {
          return MeshFormat::GMSH;
        }
        else
        {
          throw unknown_format{"Could not determine format of " + meshfile.string()};
        }
      }

     private:
      /**
       * Check that the mesh file does, in fact, exist.
       *
       * Throws a `std::filesystem::filesystem_error` if not.
       */
      void check_mesh_exists(const std::filesystem::path& meshfile) const
      {
        std::error_code ec;
        if (!std::filesystem::exists(meshfile, ec))
        {
          throw std::filesystem::filesystem_error{"Could not find mesh file " + meshfile.string(), ec};
        }
      }
    };

  }  // namespace reader
}  // namespace cfg

#endif  // __CFG_DETECT_FORMAT_H_
