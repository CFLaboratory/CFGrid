/**
 * topo_parser.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <functional>
#include <iostream>

#include <_topo_parser.h>
#include <topo_parser.h>

namespace cfg::parser
{
  std::ostream& operator<<(std::ostream& os, const Topo& topo)
  {
    return os << "Topo:\n";
  }

  class DataParser
  {
   public:
    Topo parse(const reader::SectionReader& topo_reader,
               std::ifstream& mesh_stream,
               const reader::Mode mode,
               const TopoHeader& hdr,
               const std::vector<Node<3>>& nodes) const
    {
      return {};
    }
  };

  class Validator
  {
   public:
    void validate(const Topo& topo, const TopoHeader& hdr) const {};
  };

  std::function<Topo(const cfg::reader::SectionReader&, std::ifstream&, const reader::Mode)> make_topo_reader(
      const std::vector<Node<3>>& nodes)
  {
    return read_X(HeaderParser{}, DataParser{}, nodes, Validator{});
  }

  [[nodiscard]] Topo read_topo(std::ifstream& mesh_stream,
                               const reader::Mode mode,
                               const std::vector<Node<3>>& nodes,
                               const cfg::utils::Parallel& parallel)
  {
    std::cout << "+ Reading topology" << std::endl;
    const cfg::reader::SectionReader topo_reader("Elements", mesh_stream);

    // Read the topology
    const auto reader = make_topo_reader(nodes);
    const auto topo   = reader(topo_reader, mesh_stream, mode);

    // Check that we read the Elements section correctly -> we should read "$EndElements"
    std::string line;
    topo_reader(mesh_stream) >> line;
    if (line != "$EndElements")
    {
      throw std::runtime_error("The Elements (topology) section was ready incorrectly");
    }

    return topo;
  }
}  // namespace cfg::parser
