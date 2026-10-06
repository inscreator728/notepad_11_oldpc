# Notepad (tabbed)

A small, dependency-free tabbed text editor for Windows, written in portable
Win32 C++11. It aims to behave like the classic Windows Notepad — including its
encoding handling, keyboard shortcuts and printing — while adding multiple
document tabs.

![Screenshot](screenshot.png)

*One translation unit, no third-party libraries, no installer: the whole
program is `main.cpp` plus a resource file.*

---

## Contents

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
- [Running](#running)
- [Testing](#testing)
- [Repository layout](#repository-layout)
- [How the code is organised](#how-the-code-is-organised)
- [Text encodings](#text-encodings)
- [High-DPI and visual styles](#high-dpi-and-visual-styles)
- [Security](#security)
- [Bugs fixed](#bugs-fixed)
- [Known limitations](#known-limitations)
- [Contributing](#contributing)
- [License](#license)

---

## Features

**Menus**

| Menu | Items |
|------|-------|
| **File** | New Tab `Ctrl+N` · Open `Ctrl+O` · Save `Ctrl+S` · Save As `Ctrl+Shift+S` · Close Tab `Ctrl+W` · Page Setup · Print `Ctrl+P` · Exit |
| **Edit** | Undo `Ctrl+Z` · Cut `Ctrl+X` · Copy `Ctrl+C` · Paste `Ctrl+V` · Delete `Del` · Find `Ctrl+F` · Find Next `F3` · Replace `Ctrl+H` · Go To `Ctrl+G` · Select All `Ctrl+A` · Time/Date `F5` |
| **Encoding** | ANSI · UTF-8 · UTF-8 with BOM · UTF-16 LE · UTF-16 BE |
| **View** | Word Wrap · Status Bar · Next Tab `Ctrl+Tab` · Previous Tab `Ctrl+Shift+Tab` |
| **Help** | About Notepad |

**Behaviour**

- Unlimited tabs; the active tab's name (with a `*` when dirty) drives both the
  tab title and the window title.
- Drag-and-drop a file onto the window to open it; several files may be passed
  on the command line and each gets its own tab.
- Word wrap is a per-view toggle that preserves the caret and scroll position.
- The status bar reports `Ln/Col`, character count, line-ending style and
  encoding.
- Find / Replace are modeless and keep their search history across openings;
  Go To Line is a modal dialog built from an in-memory dialog template (no
  `.rc` dialog resource is needed).
- Each tab has an owner-drawn close button that is debounced, so a double
  click only closes one tab.
- Print and Page Setup honour the configured margins and paginate by measuring
  each line before drawing it.

## Requirements

- **Windows 7 SP1 or later**, through Windows 11.
- To *build*: one of the toolchains in the next section. There are no
  third-party libraries — only `windows.h`, `commctrl.h`, `commdlg.h`,
  `shellapi.h` and `strsafe.h`, all of which ship with every Win32 SDK.

## Building

### Windows — MSYS2 / MinGW-W64

```sh
pacman -S mingw-w64-ucrt-x86_64-gcc make    # once
mingw32-make
```

### Linux — mingw-w64 cross compiler

```sh
sudo apt install mingw-w64 g++ make          # Debian/Ubuntu
make
```

The Makefile detects the operating system and picks `g++` / `windres` on
Windows, or `x86_64-w64-mingw32-g++` / `x86_64-w64-mingw32-windres` on Linux.
Both produce the same native 64-bit `Notepad.exe`.

| Switch | Effect |
|--------|--------|
| `make NO_RES=1` | Build without the embedded manifest (no v6 theming or DPI declaration). `main.cpp` still calls `SetProcessDPIAware()`, so the layout stays correct. |
| `make CXX=... WINDRES=...` | Override the tool names if yours are different. |
| `make clean` | Remove build outputs. |

### Windows — MSVC

Run `build_msvc.bat` from a **Developer Command Prompt for VS** (it needs
`cl.exe` and `rc.exe` on `PATH`). It compiles `app.rc` first, then links with
`/MANIFEST:NO` because the resource already carries manifest id 1.

> Objects are always placed before the libraries on the link line and the
> import libraries are listed explicitly, so the same Makefile works with
> toolchains that pull in Win32 libraries implicitly and with those that do
> not.

## Running

```
Notepad.exe                     # one empty tab
Notepad.exe a.txt b.txt         # one tab per file
Notepad.exe "C:\path\file.log"
```

Close a tab with its `✕` or `Ctrl+W`. Closing the last tab exits the program.
Unsaved tabs prompt for a save before the application quits.

## Testing

Three PowerShell harnesses drive the real executable through the Win32 API —
they mock nothing. Each writes its own fixtures, so they are safe to run
repeatedly and safe to delete.

```powershell
powershell -ExecutionPolicy Bypass -File verify_decode.ps1   # load encodings
powershell -ExecutionPolicy Bypass -File verify_save.ps1     # save round-trips
powershell -ExecutionPolicy Bypass -File verify_ui.ps1       # layout / commands
powershell -ExecutionPolicy Bypass -File screenshot.ps1      # writes screenshot.png
```

| Script | Checks |
|--------|--------|
| `verify_decode.ps1` | 6 cases — ASCII, UTF-8 with and without BOM, UTF-16 LE with BOM, UTF-16 LE/BE *without* BOM all decode byte-exact. |
| `verify_save.ps1` | 5 cases — load → edit → save round-trips are byte-identical for ANSI, UTF-8, UTF-8 BOM, UTF-16 LE and UTF-16 BE. |
| `verify_ui.ps1` | 6 cases — file load, word-wrap round trip preserves the document, status-bar toggle, dirty-flag propagation, open and close tab. |

All three exit `0` on success and print a `FAILURES=` line.

> **Note for anyone writing a new capture test:** call
> `SetProcessDPIAware()` in the harness *before* measuring. Windows hands a
> DPI-unaware process virtualised coordinates while `CopyFromScreen` reads
> physical pixels — mixing the two silently samples the wrong rectangle.

## Repository layout

```
main.cpp            the whole program (~1,900 lines)
app.rc              embeds app.manifest as RT_MANIFEST (id 1)
app.manifest        comctl32 v6, supportedOS, system DPI awareness
Makefile            Windows + Linux builds
build_msvc.bat      MSVC build
verify_decode.ps1   encoding-decode tests
verify_save.ps1     encoding-save tests
verify_ui.ps1       layout / command tests
screenshot.ps1      produces screenshot.png
```

## How the code is organised

`main.cpp` is a single translation unit grouped into rough layers:

1. **Dynamic API loading.** `LoadDynamicApis()` resolves every GDI / COMMDLG /
   COMCTL32 entry point with `GetProcAddress`, and the macros below it redirect
   the corresponding names to those pointers.
2. **Globals and helpers** — the tab vector, active index, menu and command
   IDs, and the `Scale()` DPI helper.
3. **Tab model** — `AddNewTab`, `RemoveTab`, `SelectTab`, `SaveTab`,
   `ConfirmSave`, `OpenPath`.
4. **Encoding layer** — `DecodeFileBytes`, `EncodeText`, `DetectUtf16NoBom`,
   `NormalizeNewlines`.
5. **Search** — `SearchEdit`, `ReplaceCurrent`, `ReplaceAllInEdit` and the
   Find/Replace dialog plumbing.
6. **Layout and painting** — `LayoutChildren`, `DrawTabItem`,
   `UpdateStatusBar`.
7. **Dialogs** — `ShowGotoDialog` builds a `DLGTEMPLATE` in memory; printing
   lives in `PrintText`, `DoPrint` and `DoPageSetup`.
8. **`WndProc`**, then **`WinMain`** (wrapped in `RunNotepad` plus a top-level
   exception guard).

### Layout invariant

`LayoutChildren()` maintains three non-overlapping horizontal bands:

```
+---------------------------+
| tab strip      (0..strip) |
+---------------------------+
| editors        (strip..)  |
+---------------------------+
| status bar     (..bottom) |
+---------------------------+
```

This matters: the tab control and the editors are *siblings* and both carry
`WS_CLIPSIBLINGS`. If the tab control were sized to the whole client area, the
editor would lie entirely inside the tab's rectangle, be clipped out of every
paint, and the document would never appear. The code asks `TCM_ADJUSTRECT` how
tall the strip is and sizes both controls from that.

## Text encodings

Detection order when opening a file:

1. **BOM present** → UTF-8, UTF-16 LE or UTF-16 BE.
2. **No BOM** → `DetectUtf16NoBom()` looks at where the zero high bytes fall:
   an odd byte index means little endian, an even index means big endian.
   Without this, UTF-16 files saved by older tools were decoded as UTF-8/ANSI
   and arrived in the editor as interleaved garbage.
3. **Strict UTF-8 validation** (`MB_ERR_INVALID_CHARS`) → UTF-8.
4. Otherwise → the system ANSI code page.

The encoding can be changed from the **Encoding** menu at any time; the choice
is stored on the tab and used on the next save. Line endings are preserved
verbatim — the edit control stores CRLF internally, which is exactly what the
status bar reports.

Files larger than **100 MiB** (`MAX_FILE_BYTES`) are refused with a clear
message instead of being read into memory.

## High-DPI and visual styles

- `SetProcessDPIAware()` is called before any window or font is created, and
  `app.manifest` declares the same, so the process works in real pixels.
- `g_dpi` is measured once; every hard-coded pixel constant goes through
  `Scale()`, so the tab strip, the close button, the status-bar partition
  widths and the initial window size all track the display scale.
- Fonts are created at the measured DPI instead of using the stock
  `DEFAULT_GUI_FONT`, which is baked at 96 DPI and renders far too small on a
  scaled display.
- `app.manifest` also activates common controls v6 (themed tabs, status bar
  and common dialogs), the `supportedOS` entries for Vista through Windows 11,
  and the `asInvoker` execution level.

## Security

The threat model and the full list of mitigations are in
[SECURITY.md](SECURITY.md). In summary:

- System DLLs are loaded with `LOAD_LIBRARY_SEARCH_SYSTEM32`, with an explicit
  `GetSystemDirectoryW` fallback for Windows 7 without KB2533623, so a planted
  `gdi32.dll` next to the executable cannot be picked up.
- No unbounded formatting: `wsprintf` was replaced by `StringCchPrintfW`, and
  the in-memory dialog-template writer is bounds checked against its buffer.
- Every file read goes through `GetFileSizeEx` under a hard size cap.
- Load, save and the whole message loop are guarded so a `std::bad_alloc`
  surfaces as an error dialog instead of reaching `std::terminate()`.
- Command-line and drag-and-drop paths are opened with `CreateFileW` directly —
  no shell, therefore no command injection.

## Bugs fixed

Recorded here because each one makes a good regression test:

| Area | Problem |
|------|---------|
| Layout | The tab control was sized to the entire client area; with `WS_CLIPSIBLINGS` on both siblings the editor was clipped out of every paint, so **document text never appeared**. |
| Layout | The status bar relied solely on its own `WM_SIZE` handler. It is now positioned explicitly, guarded against a zero-height measurement, raised above the editors and repainted after each layout pass. |
| Encoding | BOM-less UTF-16 files were decoded as garbage; the detection's endianness was inverted on the first attempt and the round-trip tests caught it. |
| Tabs | Double-clicking a tab's `✕` delivered `NM_CLICK` twice at the same point and silently closed a second, adjacent tab. |
| Dirty state | Toggling word wrap recreated the edit control and lost its modify flag, desynchronising it from the tab. |
| Printing | Font and DC state were applied *before* `StartPage` (which resets the DC), `StartPage`'s return value was ignored, and `EndPage` could run after a failed `StartPage`. |
| Printing | `PrintDlg` can free the `hDevMode`/`hDevNames` handles even when it returns `FALSE`, but `g_psd` was only synced on success — leaving a dangling `HGLOBAL` for the next Page Setup. |
| Accelerators | Menu accelerators fired while the caret was inside the modeless Find/Replace box, so `Ctrl+A` selected all in the *editor* and `F5` inserted the date into the document. |
| Search | `std::wstring::find` could be called with `pos > size()` and throw; Replace All called `lstrlen(NULL)`. |
| Startup | `GetMessage` was looped with `> 0`, treating an error as a normal quit and then returning an uninitialised `msg.wParam`. |
| Robustness | Missing control/menu creation checks, unowned error dialogs, command-line files opened before the window existed, leaked `HMODULE`s, a status bar left describing the tab it had just closed. |
| DPI | The process was DPI-unaware (bitmap-scaled and blurry) and used a 96-DPI stock font. |

## Known limitations

- **Windows only.** This is a native Win32 program; there is no Linux or macOS
  port. On Linux the project *builds* with the mingw-w64 cross compiler and
  produces a Windows `.exe`, which runs under Wine.
- The process is **system-DPI aware**, not per-monitor-v2, so it does not
  handle `WM_DPICHANGED`; Windows bitmap-scales the window when it is dragged
  onto a monitor with a different scale factor.
- BOM-less UTF-16 is detected from its zero-byte pattern, so a file written
  entirely in non-ASCII characters (for example pure CJK) with no BOM is not
  recognised.
- Undo history does not survive a word-wrap toggle, because the underlying
  edit control must be recreated to change `WS_HSCROLL`.
- `-Wconversion` still reports a few sign-conversion warnings in the menu
  handling code. `-Wall -Wextra -Wpedantic -Wshadow` are clean.

## Contributing

1. Keep the build warning-free: `mingw32-make` and the strict
   `-Wall -Wextra -Wpedantic -Wshadow` check should both stay silent.
2. Run all three `verify_*.ps1` scripts and confirm they print `FAILURES=0`.
3. If you change layout or encoding behaviour, add a case to the relevant
   harness instead of verifying it by eye.
4. Prefer the Win32 API directly — the project deliberately has no
   third-party dependencies.

## License

This repository does not yet declare a license. If you intend to distribute
it, add a `LICENSE` file (MIT, Apache-2.0 and BSD-3-Clause are all compatible
with this codebase's dependencies) and link it from this section.