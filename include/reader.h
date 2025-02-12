/**
 * reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_READER_H_
#define __CFG_READER_H_

#include <istream>

#include <section_reader.h>

namespace cfg::reader
{
  /**
   * Identifies which mode the mesh file is read in: ASCII (formatted) or BINARY (unformatted).
   */
  enum class Mode
  {
    ASCII,
    BINARY
  };

  /**
   * Reads a single item from the reader, according to the mode.
   */
  template <class T>
  [[nodiscard]] T read_one(const reader::SectionReader& section_reader, std::istream& mesh_stream, const Mode mode)
  {
    T val;
    if (mode == Mode::ASCII)
    {
      section_reader(mesh_stream) >> val;
    }
    else
    {
      const size_t nchar = sizeof(T) / sizeof(char);
      mesh_stream.read((char*)&val, nchar);
    }
    return val;
  }
}  // namespace cfg::reader

#endif  // __CFG_READER_H_
