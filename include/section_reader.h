/**
 * section_reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_SECTION_READER_H_
#define __CFG_SECTION_READER_H_

#include <istream>
#include <string>

namespace cfg::reader
{
  /**
   * A utility class template to support the implementation of `operator>>` for the `SectionReader`
   * class.
   *
   * @note This class is intended to be created as a temporary/intermediate object and not used
   *       directly by users.
   */
  template <class SR>
  class SectionReaderExtractor
  {
   public:
    /**
     * Constructs a `SectionReaderExtractor` for a `SectionReader` and associated `std::istream`
     * object.
     *
     * @param sr The `SectionReader` object that is reading a section from the `std::istream`.
     * @param is The `std::istream` that is being read.
     */
    SectionReaderExtractor(const SR& reader, std::istream& stream) : reader(reader), stream(stream) {}

    /**
     * Extraction operator, extracts an item from the underlying stream.
     *
     * @param str The destination to extract the item into.
     * @returns   The stream reference.
     */
    template <class T>
    std::istream& operator>>(T& val)
    {
      return reader.pop_word(stream, val);
    }

   private:
    // Generally we should not capture a reference as part of a class/structure, however this is a
    // temporary object that exists only to implement the operator>> for SectionReader and this
    // temporary capture should be OK.
    const SR& reader;      // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::istream& stream;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
  };

  /**
   * Provides a stream-like interface to read a section of a GMSH from a stream.
   *
   * GMSH file sections are bracketed by `$NAME` and `$EndNAME` lines and these are used to identify
   * the limits of the section stream.
   */
  class SectionReader
  {
   public:
    /**
     * Constructs a `SectionReader` object.
     *
     * @param section_name The identifier for the section, i.e. this will read between
     *                     `$section_name` and `$Endsection_name` in the GMSH file.
     * @param mesh_data Stream to read mesh from.
     */
    SectionReader(const std::string& section_name, std::istream& mesh_data)
        : start_sygil("$" + section_name), end_sygil("$End" + section_name)
    {
      // Searches the mesh for the `section_name` sygil.
      auto search = [&mesh_data](const std::string& start_sygil) -> std::istream::pos_type
      {
        std::string word;
        bool found = false;
        auto pos   = mesh_data.tellg();
        while (mesh_data >> word)
        {
          if (word == start_sygil)
          {
            found = true;
            break;
          }

          // Update location
          pos = mesh_data.tellg();
        }

        if (!found)
        {
          throw std::runtime_error("Couldn't find mesh section " + start_sygil);
        }

        return pos;
      };

      // Locate the start of our mesh section, note that we might have already read past the
      // section.
      try
      {
        start = search(start_sygil);
      }
      catch (const std::runtime_error& e)
      {
        // Try again from the start
        mesh_data.seekg(0);
        start = search(start_sygil);
      }
    }

    /**
     * Seeks a location of the mesh stream, relative to the beginning of the section, resetting any
     * flags.
     *
     * @param mesh_data The mesh stream.
     * @param pos The position to go to, reltive to section start: e.g. `pos=0` would go to the
     *            start of the section.
     */
    void seekg(std::istream& mesh_data, const std::istream::pos_type pos) const
    {
      clear(mesh_data);
      mesh_data.seekg(start + pos);
    }

    /**
     * Pops a word or item from the section, if the end of section is found sets the stream to
     * `EOF`.
     *
     * @param mesh_data The stream being read.
     * @param val       The destination the word is inserted into.
     * @returns         The stream.
     */
    template <class T>
    [[nodiscard]] std::istream& pop_word(std::istream& mesh_data, T& val) const
    {
      ffwd(mesh_data);
      mesh_data >> val;

      return mesh_data;
    }
    [[nodiscard]] std::istream& pop_word(std::istream& mesh_data, std::string& val) const
    {
      ffwd(mesh_data);
      mesh_data >> val;

      if (is_section_end(val))
      {
        set_end(mesh_data);
      }
      else if (mesh_data.eof())
      {
        throw std::runtime_error{"Read to EOF without finding section end"};
      }

      return mesh_data;
    }

    /**
     * Constructs a temporary `SectionReaderExtractor` to implement `operator>>`.
     *
     * @param mesh_data The stream being read.
     * @returns         A (temporary) `SectionReaderExtractor`.
     */
    [[nodiscard]] SectionReaderExtractor<SectionReader> operator()(std::istream& mesh_data) const
    {
      return {*this, mesh_data};
    }

    /**
     * Reads a line from the mesh stream, returning as a function.
     *
     * @param mesh_data The stream being read.
     * @returns         The current line.
     *
     * @note The `std::getline` calling convention takes `line` as a return parameter, however it
     *       seems more natural to return it directly. The `std::getline` calling convention could
     *       be implemented as an overload of this if required.
     */
    [[nodiscard]] std::string getline(std::istream& mesh_data) const
    {
      std::string line;
      std::getline(mesh_data, line);

      if (is_section_end(line))
      {
        set_end(mesh_data);
      }

      return line;
    }

   private:
    std::string start_sygil;       // Identifies the start of the section
    std::string end_sygil;         // Identifies the end of the section
    std::istream::pos_type start;  // Locates the start of the section in the stream

    /**
     * Clears any status flags on the stream itself.
     *
     * @param mesh_data The mesh stream.
     */
    // This could be static, but the idea of the SectionReader is to have some state wrt the section
    // being read, and this would defeat the purpose.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void clear(std::istream& mesh_data) const
    {
      mesh_data.clear();  // Reset data stream flags
    }

    /**
     * Fast-forwrads the stream to the beginning of the section.
     *
     * @param mesh_data The mesh stream.
     */
    void ffwd(std::istream& mesh_data) const
    {
      if (mesh_data.tellg() < start)
      {
        seekg(mesh_data, 0);
      }
    }

    /**
     * Tests whether the end of the section was read.
     *
     * @param str The string to test.
     * @returns   Flag indicating whether the end of the section was read (`true`) or not (`false`).
     */
    [[nodiscard]] bool is_section_end(const std::string& str) const
    {
      return str == end_sygil;
    }

    /**
     * Sets the EOF condition on the stream, indicating the end of section was reached.
     *
     * @param mesh_data The mesh stream.
     */
    // This could be static, but the idea of the SectionReader is to have some state wrt the section
    // being read, and this would defeat the purpose.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void set_end(std::istream& mesh_data) const
    {
      mesh_data.seekg(0, std::ios::end);
    }
  };
}  // namespace cfg::reader

#endif  // __CFG_SECTION_READER_H_
