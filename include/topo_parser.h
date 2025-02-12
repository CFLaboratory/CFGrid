/**
 * topo_parser.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CFG_TOPO_PARSER_H_
#define __CFG_TOPO_PARSER_H_

#include <fstream>

#include <node_parser.h>
#include <utils.h>

namespace cfg::parser
{
  class Topo
  {
  };

  std::ostream& operator<<(std::ostream& os, const Topo& topo);

  /**
   * Reads the topology from a mesh file
   */
  [[nodiscard]] Topo read_topo(std::ifstream& mesh_stream,
                               const reader::Mode mode,
                               const std::vector<Node<3>>& nodes,
                               const cfg::utils::Parallel& parallel);
}  // namespace cfg::parser

#endif  // __CFG_TOPO_PARSER_H_
