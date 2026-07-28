# Architecture

libmsftrace separates physical container decoding from logical PDB interpretation and derived query construction.

1. **Context and reader** establish bounded little-endian access and error reporting.
2. **Superblock and directory** validate MSF page geometry and materialize logical streams.
3. **PDB information** loads identity metadata and the named-stream map.
4. **Type streams** enumerate TPI and IPI records and classify CodeView leaves.
5. **DBI modules** establish module ownership and section contributions.
6. **Symbol streams** decode module, global, and public records.
7. **Source metadata** joins file tables, checksums, C13 line blocks, and inline annotations.
8. **Address metadata** normalizes section maps, OMAP segments, contribution intervals, and frame programs.
9. **Validation** checks cross-stream identities and records availability by query domain.
10. **Derived indexes** build canonical type, identifier, symbol, source, frame, and named-stream views.
11. **Queries and formatting** provide stable read-only results to callers.

Physical records retain their stream-local identities. Canonical objects use numeric identities so that query views can be reconstructed after normalization or validation. The semantic modules are independent building blocks used by higher-level parsing and diagnostic tools.
