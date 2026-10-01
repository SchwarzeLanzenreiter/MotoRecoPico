# FatFs (ChaN) — vendored

The generic FAT filesystem module from ChaN, **FatFs R0.16** (July 2026), taken unmodified from
the official release `arc/ff16.zip` at <http://elm-chan.org/fsw/ff/>. `LICENSE.txt`,
`00readme.txt` and `00history.txt` are the ones shipped with it.

## What is here

From the release's `source/` directory:

| File | Purpose |
|---|---|
| `ff.c` | the filesystem itself |
| `ff.h` | its API |
| `ffunicode.c` | code page 437 conversion, required because `FF_USE_LFN` is on |
| `ffsystem.c` | `ff_mem_alloc` / mutex stubs. only the stubs are used (`FF_FS_REENTRANT 0`) |
| `diskio.h` | the block device interface `src/diskio.c` implements |

The release's own `ffconf.h` is deliberately **not** copied. This project's configuration lives in
[`src/ffconf.h`](../../src/ffconf.h) and is on the include path ahead of this directory. Its
`FFCONF_DEF` must keep matching `FF_DEFINED` in `ff.h` (80386 for R0.16) or `ff.c` refuses to
compile — check that first when upgrading.

`CMakeLists.txt` stops the build with a clear message if `ff.c` is missing.

## Why vendored rather than fetched

The build must work with no network. FatFs is a single small module with a stable API and a
permissive one-clause licence, so carrying it is cheaper than a submodule or `FetchContent`.

## Configuration that matters

`src/ffconf.h` differs from the stock template in three places that the logger depends on:

- `FF_USE_LFN 1` — `20260731_120423.dat` is 19 characters and cannot be a short 8.3 name. Without
  long names the files would not match the Raspberry Pi version's naming.
- `FF_FS_NORTC 0` — timestamps come from `get_fattime()` in `src/diskio.c`, which reads the
  GPS-derived clock, so files carry a real date once the receiver has a fix.
- `FF_FS_EXFAT 0`, `FF_MAX_SS 512` — FAT32 only, fixed sector size. Format cards on a PC.
