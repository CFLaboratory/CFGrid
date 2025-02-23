/**
 * node_parser.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <_node_parser.h>

#include <iostream>

namespace cfg::parser
{
  void read_nodes(std::ifstream& mesh_stream, const Mode mode, const cfg::utils::Parallel& parallel)
  {
    std::cout << "+ Reading nodes" << std::endl;
    const cfg::reader::SectionReader node_reader("Nodes", mesh_stream);

    // Read the nodes
    const auto reader = make_node_reader(parallel);
    const auto nodes  = reader(node_reader, mesh_stream, mode);

    // Check that we read the Nodes section correctly -> we should read "$EndNodes"
    std::string line;
    node_reader(mesh_stream) >> line;
    if (line != "$EndNodes")
    {
      throw std::runtime_error("The Nodes section was read incorrectly");
    }

    // Report how many nodes we read
    std::cout << "++ Rank " << parallel.rank << " read " << nodes.size() << " nodes" << std::endl;
  }
}  // namespace cfg::parser
