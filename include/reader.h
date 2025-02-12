/**
 * reader.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_READER_H_
#define __CFG_READER_H_

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
}  // namespace cfg::reader

#endif  // __CFG_READER_H_
