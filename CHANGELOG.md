# CHANGELOG

This changelog tracks the development of `CFGrid`.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
### Changed

- Refactored the `read_nodes` function in terms of a generic `read_X` function
- Replaced `parse_node_header` with `HeaderParser::parse`
- Replaced `parse_node_blocks` with `DataParser::parse`
- The `Node.idx` field was replaced by `Node.natural_idx` and a `Node.global_idx` field added

### Deprecated
### Removed
### Fixed

## [0.1] - 2025-02-04

### Added

- Implemented parallel (MPI-only) node reading for GMSH meshes
- Added a `CONTRIBUTING.md` guide
- Added clang-format and clang-tidy configurations to support consistent code style
- Added `Catch2`-based testing
- Added a `CMake`-based build system
- Renamed project from `CFMesh` -> `CFGrid` to avoid confusion with an existing mesh generator
- Added a style guide
- Added a Doxygen-based documentation system
- Licensed under Apache 2.0

### Changed
### Deprecated
### Removed
### Fixed

