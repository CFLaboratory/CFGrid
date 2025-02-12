/**
 * main.cpp
 */

#include <sysexits.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include "utils.h"

#include <mpi.h>

#include <detect_format.h>
#include <gmsh.h>
#include <node_parser.h>
#include <reader.h>
#include <section_reader.h>

/**
 * Converts the `{argc, argv}` tuple into a vector of arguments.
 *
 * @param argc The argument count.
 * @param argv The array of argument strings, `argv[0]` is the program name.
 * @returns    A vector of argument strings, dropping the program name.
 */
// We are converting the argv char* array to a vector so we cannot avoid the use of C-style arrays
// here.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
[[nodiscard]] std::vector<std::string> get_argvector(int argc, char* argv[])
{
  if (argc < 2)
  {
    throw std::runtime_error("CFGrid requires at least one argument - the mesh file to read");
  }

  // Extract arguments, dropping program name (argv[0])
  std::vector<std::string> args(argc - 1);
  for (int i = 1; i < argc; i++)
  {
    // Apparently in C++20 we could use a span to resolve this, but currently we are trying to stick
    // to C++17.
    args[i - 1] = std::string(argv[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  return args;
}

void read_mesh(const std::filesystem::path& mesh_file, const cfg::utils::Parallel& parallel)
{
  std::cout << "Reading mesh file: " << mesh_file << std::endl;
  if (cfg::reader::FormatDetector::get_format(mesh_file) == cfg::reader::MeshFormat::GMSH)
  {
    cfg::mesh::GmshReader reader(mesh_file, parallel);
  }
  else
  {
    throw std::runtime_error("CFGrid only supports reading GMSH files currently");
  }
}

void chkerr(const int ierr)
{
  if (ierr != 0)
  {
    throw std::runtime_error("MPI raised an error");
  }
}

int main(int argc, char* argv[])
{
  // Initialise parallelism
  int size            = 0;
  int rank            = 0;
  int ierr            = 0;
  ierr                = MPI_Init(&argc, &argv); chkerr(ierr);
  ierr                = MPI_Comm_size(MPI_COMM_WORLD, &size); chkerr(ierr);
  ierr                = MPI_Comm_rank(MPI_COMM_WORLD, &rank); chkerr(ierr);
  const auto parallel = [rank, size]() -> cfg::utils::Parallel
  {
    cfg::utils::Parallel parallel{};
    parallel.rank = rank;
    parallel.size = size;
    return parallel;
  }();

  // Parse args
  const auto args = get_argvector(argc, argv);
  std::filesystem::path mesh_file(args[0]);

  read_mesh(mesh_file, parallel);

  ierr = MPI_Finalize(); chkerr(ierr);

  return 0;
}
