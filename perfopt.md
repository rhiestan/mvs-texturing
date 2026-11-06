# Performance optimizations (`perfopt` branch)

This fork of [MVS-Texturing](https://github.com/nmoehrle/mvs-texturing) adds
a modern build for `texrecon` and performance optimizations. The work lives
on the `perfopt` branch (the default branch of this fork); `master` tracks the
upstream repository, so it can be synchronized with it.

Timings below were measured on a 32-thread Windows machine (Clang 22) with an
11-view scene of 839k faces.

## Build and CI

- CMake, Ninja, Clang and vcpkg (Eigen, TBB, image libraries) build
  `texrecon`; MVE, rayint and mapmap come from `FetchContent` instead of
  `ExternalProject`. The sources were adapted to the current MVE API.
  Presets: `windows-clang`, `linux-clang`, `macos-clang`.
- GitHub Actions builds on Windows, Linux and macOS (replacing the obsolete
  Travis configuration). Pushing a tag packages `texrecon` (with the OpenMP
  runtime) and publishes a GitHub release.

## Optimizations

- **mimalloc** for all C++ allocations. texrecon allocates heavily, mostly in
  the MRF view selection and the adjacency graph: 39 s went down to 25 s.
- **`apply_edge_padding` and `prepare_blending_mask`** allocated a
  `std::set` node per pixel. They now use a flat list with a bitmap. The
  visiting order does not influence the result; the output is byte-identical
  in single-threaded runs.
