# libmsftrace

libmsftrace is a C11 library for inspecting Microsoft MSF 7.0 containers and PDB debug databases. It materializes stream directories, decodes selected CodeView type and symbol records, resolves module and source metadata, normalizes address maps, and exposes query APIs suitable for command-line tools and build diagnostics.

## Features

- checked MSF superblock, page-map, and stream-directory parsing;
- PDB information and named-stream decoding;
- TPI/IPI record enumeration and CodeView leaf catalogues;
- DBI module, symbol, source-line, OMAP, and frame extraction;
- interval, hash, path, GUID, UTF-8, numeric-leaf, and vector utilities;
- semantic algorithms for type selection, field continuations, identifiers, module state, contributions, public symbols, GSI chains, OMAP, source identity, inline sites, frames, and named streams;
- four libFuzzer harnesses and sanitizer-friendly build scripts.

## Build

```sh
make
make test
```

The public header is `src/msftrace.h`. Internal semantic facilities are declared in `src/semantic.h` for the inspection tool and focused validation programs.
