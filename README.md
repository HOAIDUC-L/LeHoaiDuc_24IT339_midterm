# Midterm Project: Simplified UNIX `ls(1)` Implementation in C

## 1. Student Information

* **Student Name:** Le Hoai Duc
* **Student ID:** 24IT339
* **Class / Subject:** Linux & Open Source Software / Advanced Systems Programming
* **Project:** Midterm Project – Implementation of UNIX `ls(1)` Command from Scratch
* **Repository:** `LeHoaiDuc_24IT339_midterm`
* **GitHub URL:** [https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm](https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm)

---

## 2. Project Description

This project is a clean, modular, and standards-compliant implementation of the UNIX `ls(1)` utility written in C (C99 / POSIX.1-2008). It is built entirely from scratch without relying on external non-standard libraries or GNU-specific extensions.

### Scope & Specification
The project is strictly designed against the provided **NetBSD 10.1 `ls(1)` manual page specification** (`ls [ -AacdFfhiklnqRrSstuw] [file ...]`). In compliance with project guidelines:
* Only options, behaviors, formatting styles, and precedence rules documented in the manual page are implemented.
* The default display format outputs one entry per line to standard output, adhering to the specification: *"By default, ls lists one entry per line to standard output."*
* Undocumented options or GNU-specific behaviors are omitted by design.
* Robust error handling ensures graceful continuation when encountering missing files, permission errors, or broken symbolic links.

### Core UNIX / POSIX Concepts Applied
* **Filesystem Traversal & Directory I/O:** `opendir()`, `readdir()`, `closedir()`, entry filtering (`.` and `..`).
* **File Metadata & Inode Operations:** `stat()`, `lstat()`, `readlink()`, file types (regular files, directories, symbolic links, character/block special devices, FIFOs, sockets), file permission bits (`mode_t`, `st_mode`), setuid, setgid, sticky bit, and inode number extraction.
* **User & Group Database Resolution:** `getpwuid()`, `getgrgid()`, translating numeric IDs to names, with fallback to numeric formatting.
* **Terminal & Stream Detection:** `isatty(STDOUT_FILENO)` for terminal-aware defaults (`-q` vs `-w`, total block headers for `-s`).
* **Environment Configuration:** Parsing and application of the `BLOCKSIZE` environment variable.
* **Deterministic Sorting & Data Algorithms:** Multi-key sorting with `qsort()`, reverse sorting, time/size criteria, and strict tie-breaking.
* **Clean Memory Management:** Dynamic data structures, verified leak-free with AddressSanitizer and UndefinedBehaviorSanitizer.

---

## 3. Implemented Features

Every single option from the provided manual page specification synopsis `ls [ -AacdFfhiklnqRrSstuw] [file ...]` is fully implemented:

| Option | Description | Implementation Details & Overrides |
| :--- | :--- | :--- |
| `-A` | List all entries except for `.` and `..` | Automatically set for super-user (`geteuid() == 0`). Overridden by `-a` if `-a` is also set. |
| `-a` | Include directory entries starting with `.` | Displays all hidden entries including `.` and `..`. |
| `-c` | Use status change time (`st_ctime`) | Modifies `-t` (sorting) and `-l` (display). Overrides and is overridden by `-u`. |
| `-d` | Treat directories as plain files | Do not traverse directories; do not indirect through symlink arguments. Overrides and is overridden by `-R`. |
| `-F` | File type classification indicators | Appends `/` (dir), `*` (executable), `@` (symlink), `=` (socket), `\|` (FIFO), `%` (whiteout). |
| `-f` | Disable sorting | Output is listed in directory read order without sorting. |
| `-h` | Human-readable sizes | Modifies `-s` and `-l` to format sizes in powers of 1024 (`B`, `K`, `M`, `G`, etc.). Overrides and is overridden by `-k`. |
| `-i` | Inode number | Prints file serial number (inode) before file details. |
| `-k` | Sizes in kilobytes | Modifies `-s` to report block counts in 1024-byte units. Overrides and is overridden by `-h`. |
| `-l` | Long listing format | Displays mode, links, owner, group, size (or major/minor devices), date/time, and pathname (`-> target` for symlinks). Overrides and is overridden by `-n`. |
| `-n` | Numeric long listing format | Equivalent to `-l`, but displays numeric owner and group IDs instead of resolving names. Overrides and is overridden by `-l`. |
| `-q` | Replace non-printable characters with `?` | Default behavior when output is connected to a terminal. Overrides and is overridden by `-w`. |
| `-R` | Recursively list subdirectories | Traverses encountered subdirectories depth-first. Does not follow directory symlinks during recursion. Overrides and is overridden by `-d`. |
| `-r` | Reverse sort order | Inverts sort comparison for lexicographical, size, or timestamp sorting. |
| `-S` | Sort by file size | Sorts largest file first. Tie-break is deterministic lexicographical order. |
| `-s` | Display block count | Displays file system blocks allocated in units of 512 bytes (or `BLOCKSIZE` / 1024 if `-k`). Prints total sum before listing when outputting to terminal. |
| `-t` | Sort by timestamp | Sorts most recently modified/accessed/status-changed first. Tie-break is deterministic lexicographical order. |
| `-u` | Use access time (`st_atime`) | Modifies `-t` and `-l`. Overrides and is overridden by `-c`. |
| `-w` | Force raw printing of non-printable chars | Default behavior when output is redirected to a pipe/file. Overrides and is overridden by `-q`. |

### Option Precedence and Overrides
* **Non-printable formatting:** `-w` and `-q` override each other; the last specified flag wins.
* **Long listing ID format:** `-l` and `-n` override each other; the last specified flag wins.
* **Timestamp selection:** `-c` and `-u` override each other; default is `st_mtime`.
* **Traversal mode:** `-R` and `-d` override each other; the last specified flag wins.
* **Size scaling:** The rightmost of `-k` and `-h` overrides the previous flag.
* **Block size hierarchy:** If `-h` is active, human-readable formatting takes precedence. Else if `-k` is active, 1024-byte blocks are used. Else if `BLOCKSIZE` environment variable is set, it is used. Otherwise, standard 512-byte blocks are used.

---

## 4. Project Architecture

The codebase follows a modular design pattern with strong separation of concerns:

```text
LeHoaiDuc_24IT339_midterm/
├── Makefile                # Build configuration (strict C99 flags, clean, test)
├── README.md               # Complete project documentation and midterm report
├── .gitignore              # Ignores build artifacts, binaries, core dumps, IDE files
│
├── include/
│   ├── options.h           # Configuration structure, enum types, option parsing interface
│   ├── file_info.h         # FileInfo & FileInfoList structs, metadata resolution headers
│   ├── display.h           # Mode formatting, BSD humanize, timestamps, column alignments
│   ├── sort.h              # Multi-criterion qsort wrappers and tie-breaking algorithms
│   └── traverse.h          # Operands management, directory traversal, recursion (-R)
│
└── src/
    ├── main.c              # Program coordinator and entry point
    ├── options.c           # CLI argument parsing and precedence handling via getopt()
    ├── file_info.c         # File metadata population, symlink targets, user/group lookups
    ├── display.c           # Output rendering, column width computation, formatting routines
    ├── sort.c              # Sorting implementations (name, time, size, reverse, nosort)
    └── traverse.c          # Directory opening, readdir filtering, operand separation, recursion
```

### Module Responsibilities

1. **`main.c`:**
   * Entry point of the program.
   * Initializes default options and calls `options_parse()`.
   * Delegates execution to `traverse_operands()`.
   * Maps traversal results to the appropriate exit status (`EXIT_SUCCESS` or `EXIT_FAILURE`).

2. **`options.c` / `options.h`:**
   * Defines the `Options` structure, `SortKey` enum, and `TimeField` enum.
   * Checks runtime defaults (terminal status via `isatty()`, super-user status via `geteuid()`, and `BLOCKSIZE`).
   * Parses CLI flags with `getopt()` and implements all override logic.

3. **`file_info.c` / `file_info.h`:**
   * Defines the `FileInfo` struct containing name, full path, `struct stat`, owner, group, symlink target, and status flags.
   * Implements `FileInfoList` dynamic array with automatic capacity resizing.
   * Correctly distinguishes between `lstat()` and `stat()` for operands vs directory entries.
   * Safely reads symbolic link targets using dynamic buffer reallocation with `readlink()`.
   * Resolves owner/group names via `getpwuid()` and `getgrgid()` (or numeric representations).
   * Provides clean memory deallocation routines (`file_info_free()` and `file_info_list_free()`).

4. **`sort.c` / `sort.h`:**
   * Implements `sort_file_info_list()` using standard C `qsort()`.
   * Supports sorting by filename, modification/access/change timestamp, and file size.
   * Employs deterministic tie-breaking (using nanosecond timestamps when available, then lexicographical filename order).
   * Supports reverse ordering (`-r`) and bypasses sorting completely for `-f`.

5. **`display.c` / `display.h`:**
   * Formats POSIX file mode bits (`rwx`, setuid `s/S`, setgid `s/S`, sticky `t/T`, and file type indicator).
   * Implements NetBSD-compliant `display_humanize_number()` (powers of 1024, auto-scaling up to Exabytes).
   * Implements the NetBSD 6-month time formatting rule (`Mmm dd HH:MM` for recent files, `Mmm dd  YYYY` for older/future files).
   * Computes dynamic column widths for proper alignment across all entries.
   * Prints the `total <blocks>` header when listing directory contents in long format or block mode on a terminal.
   * Formats major and minor device numbers for block and character special devices.
   * Escapes non-printable characters when `-q` is active.

6. **`traverse.c` / `traverse.h`:**
   * Separates command-line operands into non-directory files and directories.
   * Sorts non-directory operands and displays them first, then processes directory operands.
   * Reads directories via `opendir()`, `readdir()`, and `closedir()`, filtering hidden files according to `-a` and `-A`.
   * Correctly resets `errno` before each `readdir()` call to prevent false-positive errors.
   * Formats directory header banners (`dir:\n`) when multiple operands or recursive listings occur.
   * Implements depth-first recursive traversal (`-R`), preventing infinite loops by not following directory symlinks during recursion.

---

## 5. Build Instructions

### Prerequisites
* GCC or Clang C compiler supporting C99.
* Make build system.
* Standard POSIX environment (Linux / Ubuntu).

### Compilation
To compile the project with strict warnings and flags (`-Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE`):

```bash
make
```

This compiles each `.c` source file independently into a `.o` object file under `src/` and links them into the final executable `ls`.

### Cleaning Build Artifacts
To remove compiled object files and the executable:

```bash
make clean
```

### Running the Test Suite
To build and run the automated test suite:

```bash
make test
```

---

## 6. Execution Examples

### 6.1 Basic Listing (Default: One entry per line)
```bash
./ls
```
*Output:*
```text
Makefile
include
ls
src
tests
```

### 6.2 Include Hidden Files (`-a` and `-A`)
```bash
# Include . and .. as well as hidden files
./ls -a

# Include hidden files but exclude . and ..
./ls -A
```

### 6.3 Long Listing Format (`-l` and `-n`)
```bash
# Long format with resolved user and group names
./ls -l

# Long format with numeric UIDs and GIDs
./ls -n
```
*Output (`./ls -l`):*
```text
total 88
-rw-r--r-- 1 hduc  hduc    527 Oct  7 21:26 Makefile
drwxrwxr-x 2 hduc  hduc   4096 Oct  7 21:25 include
-rwxrwxr-x 1 hduc  hduc  26960 Oct  7 21:26 ls
drwxrwxr-x 2 hduc  hduc   4096 Oct  7 21:26 src
drwxrwxr-x 2 hduc  hduc   4096 Oct  7 21:21 tests
```

### 6.4 Human-Readable File Sizes (`-h`)
```bash
./ls -lh
```
*Output:*
```text
total 44K
-rw-r--r-- 1 hduc  hduc  527B Oct  7 21:26 Makefile
drwxrwxr-x 2 hduc  hduc  4.0K Oct  7 21:25 include
-rwxrwxr-x 1 hduc  hduc   26K Oct  7 21:26 ls
drwxrwxr-x 2 hduc  hduc  4.0K Oct  7 21:26 src
drwxrwxr-x 2 hduc  hduc  4.0K Oct  7 21:21 tests
```

### 6.5 Display Blocks & Block Units (`-s`, `-k`, `BLOCKSIZE`)
```bash
# Standard 512-byte blocks
./ls -s

# 1024-byte (kilobyte) blocks
./ls -sk

# Custom block size via environment variable
BLOCKSIZE=2048 ./ls -s
```

### 6.6 Classification Indicators (`-F`)
```bash
./ls -F
```
*Output:*
```text
Makefile
include/
ls*
src/
tests/
```

### 6.7 File Inode Numbers (`-i`)
```bash
./ls -i
```
*Output:*
```text
7475084 Makefile
7475040 include
7475093 ls
7475041 src
7475042 tests
```

### 6.8 Sorting Options (`-S`, `-t`, `-r`, `-f`)
```bash
# Sort by file size (largest first)
./ls -S

# Sort by size reversed (smallest first)
./ls -Sr

# Sort by modification time (most recent first)
./ls -t

# Sort by status change time
./ls -tc

# Sort by access time
./ls -tu

# Unsorted (directory order)
./ls -f
```

### 6.9 Directory as Plain File (`-d`)
```bash
./ls -d include
./ls -ld include
```
*Output:*
```text
drwxrwxr-x 2 hduc  hduc  4096 Oct  7 21:25 include
```

### 6.10 Recursive Traversal (`-R`)
```bash
./ls -R include
```
*Output:*
```text
include:
display.h
file_info.h
options.h
sort.h
traverse.h
```

### 6.11 Multiple Operands
```bash
./ls Makefile include src
```
*Output:*
```text
Makefile

include:
display.h
file_info.h
options.h
sort.h
traverse.h

src:
display.c
display.o
file_info.c
file_info.o
main.c
main.o
options.c
options.o
sort.c
sort.o
traverse.c
traverse.o
```

---

## 7. Testing & Verification

A dedicated automated test script is provided in `tests/run_tests.sh`. It covers 15 distinct functional and edge-case test suites:

1. **Normal Directory:** Standard directory listing on directories with various file counts.
2. **Empty Directory:** Verified that `./ls empty_dir` outputs cleanly and `./ls -l empty_dir` reports `total 0`.
3. **Hidden Files:** Verified that `-a` shows `.` and `..`, `-A` shows hidden entries while omitting `.` and `..`, and default hides dotfiles.
4. **Regular Files & Directories:** Verified correct permissions and type characters (`-` and `d`).
5. **Symbolic Links:** Verified symlinks display targets formatted as `name -> target`.
6. **Broken Symbolic Links:** Tested links pointing to non-existent targets; verified program handles them safely without crashing.
7. **Multiple Operands:** Verified non-directory operands are displayed before directories, and directory operands are sorted independently.
8. **Nested Directories & Recursion:** Tested `-R` through multi-level directory structures (`dir1/subdir1`).
9. **Special Characters & Spaces:** Verified filenames with spaces (e.g. `file with spaces.txt`) and non-printable bytes (e.g. ASCII `\001`) with `-q` (`?`) and `-w` (raw).
10. **Special Device Files:** Verified character and block special files (e.g. `/dev/null`) report major and minor numbers in long format.
11. **Special Files (FIFOs):** Verified named pipes are flagged with `|` under `-F` and have type `p` in `-l`.
12. **Sorting Criteria:** Verified correct ordering across `-S`, `-Sr`, `-t`, `-tr`, and `-f`.
13. **Option Interactions:** Tested combinations such as `-lh`, `-sk`, `-li`, `-ld`, `-lc`, and `-lu`.
14. **Error Handling & Exit Status:** Verified non-existent paths print appropriate error messages to `stderr` and return exit code `1`.
15. **Memory Safety Verification:** Validated with **AddressSanitizer** and **UndefinedBehaviorSanitizer** (`-fsanitize=address,undefined`); 0 memory leaks, 0 buffer overflows, and 0 memory corruption errors detected.

---

## 8. Known Limitations

In strict adherence to the project instructions:
* **Manual Scope:** The implementation supports **only** the 19 options defined in the provided NetBSD `ls(1)` manual page (`-A`, `-a`, `-c`, `-d`, `-F`, `-f`, `-h`, `-i`, `-k`, `-l`, `-n`, `-q`, `-R`, `-r`, `-S`, `-s`, `-t`, `-u`, `-w`).
* **Multi-column Output:** Options such as `-C`, `-x`, `-m`, and `-1` are not in the provided manual specification; per the manual, output defaults to one entry per line.
* **Color Output:** Colorization (`--color` or `-G`) is GNU/BSD-specific and not present in the manual page; output uses standard text.
* **Extended Attributes & ACLs:** ACL character flags (`+`) are not included as they require non-portable platform headers outside standard POSIX.2.

---

## 9. Conclusion

This project implements a complete, robust, and standards-compliant version of the UNIX `ls(1)` utility from first principles in C. Every requirement from the course specification, modular architecture guidelines, memory management standards, and the authoritative manual page has been verified and fulfilled.
