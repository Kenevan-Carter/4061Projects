





main.c – Entry point of the program. Parses commands and dispatches to the appropriate functionality.
storage.c – Handles all disk persistence. Writes and reads file blobs to the vault, manages snapshots, and handles garbage collection of old data.
crawler.c – Scans the directory tree (BFS) to build a list of all tracked files and their metadata.
restore.c – Restores files from a previous snapshot back to disk.
stream.c – Helper utilities for reading and writing raw bytes (e.g. the write_all() helper).
mgit.h – Shared header file. Defines structs (Snapshot, FileEntry, BlockTable) and function prototypes used across all files.
Makefile – Build configuration. Compiles the project and links dependencies.
