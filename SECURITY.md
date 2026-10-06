# Security Policy

## Supported versions

Only the current `main` branch is supported. This project has no releases yet,
so please open issues and pull requests against `main`.

## Reporting a vulnerability

Please report security issues **privately** rather than in a public issue —
open a GitHub security advisory ("Report a vulnerability") or contact the
maintainers directly. Include:

- the commit hash you tested,
- a minimal file or command line that reproduces it,
- the impact you observed (crash, memory corruption, data loss, code
  execution, …).

You should get an acknowledgement within a few working days. Issues that let
an attacker-controlled *file* compromise the process are treated as high
priority; issues that merely require the user to type into the program are
not, because the program already runs with the user's full privileges.

## Threat model

`Notepad.exe` is a desktop text editor. Its security-relevant inputs are:

| Input | Source | Trust level |
|-------|--------|-------------|
| File contents | disk, chosen by the user | **untrusted** |
| File path | command line, drag-and-drop, file dialog | **untrusted** |
| Encoding / line endings | derived from file contents | **untrusted** |
| Menu and keystroke input | the user | trusted |
| Print driver / DEVMODE | installed printer | semi-trusted |

The program **runs unelevated** (`asInvoker` in the manifest), has **no
network code**, never executes, interprets or evaluates file contents, and
never shells out to another process. The realistic worst cases are therefore:

1. a malformed file crashing or corrupting the process (→ data loss),
2. a crafted file exhausting memory (→ denial of service),
3. a planted DLL being loaded instead of the system one (→ code execution).

Each of those is addressed below.

## Mitigations in place

### Preventing code execution via DLL search order

`gdi32.dll`, `comdlg32.dll` and `comctl32.dll` are resolved through
`LoadSystemLibrary()`, which calls
`LoadLibraryExW(..., LOAD_LIBRARY_SEARCH_SYSTEM32)` so only the copies in
`%SystemRoot%\System32` are considered. A `gdi32.dll` dropped next to the
executable — or in the current working directory — is therefore ignored. On
Windows 7 without KB2533623 the flag is not understood, so the fallback builds
an explicit `GetSystemDirectoryW()` path rather than dropping back to the
ambient search order.

### Memory-safety hardening

| Area | Control |
|------|---------|
| Formatting | `wsprintf` (unbounded) replaced by `StringCchPrintfW`, which always NUL-terminates and reports truncation. |
| Tab titles | The `*` dirty marker is appended only after an explicit length check instead of `lstrcat`. |
| Dialog template | `PutString`/`AddDlgItem` take a buffer end pointer and return `NULL` when anything would overflow `g_dlgBuf`. |
| File size | `GetFileSizeEx` under `MAX_FILE_BYTES` (100 MiB); larger files are refused before any allocation. |
| String conversion | Lengths are bounded by the size cap and guarded against negative values before reaching `MultiByteToWideChar` / `WideCharToMultiByte`. |
| `DrawText` length | Clamped to `INT_MAX` before the `size_t` → `int` conversion. |
| Path buffers | `GetOpenFileNameW`, `GetSaveFileNameW`, `GetFullPathNameW`, `ExpandEnvironmentStringsW` and `DragQueryFileW` are all called with an explicit capacity and the results are length-checked before use. |
| Format strings | Every format string is a literal; user text is never used as a format string. |

### Denial of service

- File size is capped before reading; the cap is enforced in one place
  (`ReadWholeFile`).
- `LoadFileIntoTab`, `SaveTab` and the entire `WinMain` body are wrapped in
  `try`/`catch`. A `std::bad_alloc` during decoding or window handling becomes
  an error dialog and a clean failure instead of `std::terminate()`, which
  previously would have killed the process — and every unsaved tab with it.

### Data integrity

- The print dialog state (`PRINTDLG`'s `hDevMode`/`hDevNames`) is synchronised
  back to `g_psd` whether `PrintDlg` succeeded or not, because the dialog may
  free those handles on failure. They are released once at exit.
- Saving uses `CreateFileW(..., CREATE_ALWAYS, ...)` on an explicit path;
  there is no intermediate file, rename or temp file that could be swapped.
- Unsaved tabs are always offered a save before the application exits, and
  cancelling that save aborts the exit rather than discarding the document.

### Injection

Paths from the command line and from `WM_DROPFILES` are passed straight to
`CreateFileW`. Nothing is ever concatenated into a shell command, so there is
no command-injection surface. There is no `WinExec`, `ShellExecute`, `system`
or `CreateProcess` call anywhere in the program.

## Residual risks and non-goals

- **No sandbox.** The program runs with the user's own privileges; opening a
  file is exactly as privileged as the user opening it themselves.
- **TOCTOU on save.** The path checked when the document was opened is reused
  when saving; an attacker able to replace that path with a symlink or junction
  in between will have the write follow it. This is inherent to "save in
  place" and is not unique to this program.
- **No virus scanning.** Files are not handed to an AV engine.
- **Large-but-legal files.** Anything at or below 100 MiB is accepted; a file
  crafted to make decoding unusually expensive can still stall the UI thread
  briefly.
- **Only C++ exceptions are guarded.** A genuine access violation still
  crashes the process; SEH handling was deliberately not added because
  `__try`/`__except` is not portable across the supported compilers.
- **No per-monitor-v2 DPI.** Not a security property, but noted in
  [README.md](README.md) for completeness.

## Verifying

```sh
make                                                    # must be warning-free
g++ -std=c++11 -Wall -Wextra -Wpedantic -Wshadow -c main.cpp
```

```powershell
powershell -ExecutionPolicy Bypass -File verify_decode.ps1
powershell -ExecutionPolicy Bypass -File verify_save.ps1
powershell -ExecutionPolicy Bypass -File verify_ui.ps1
```

All three scripts must print `FAILURES=0`. The save harness is the important
one for integrity: it proves that a document survives a full
load → edit → encode → write cycle byte-for-byte in every supported encoding.