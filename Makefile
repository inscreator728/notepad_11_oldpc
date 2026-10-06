# ---------------------------------------------------------------------------
# Cross-platform build for the tabbed Notepad clone.
#
#   Windows (MSYS2 / MinGW-W64) :  mingw32-make          -> Notepad.exe
#   Linux   (mingw-w64 cross)   :  make                  -> Notepad.exe
#   Windows (MSVC)              :  build_msvc.bat        -> Notepad.exe
#
# Both make invocations produce the same native 64-bit Windows executable.
# Override the tool names if yours differ, e.g.
#   make CXX=x86_64-w64-mingw32-g++ WINDRES=x86_64-w64-mingw32-windres
#
#   NO_RES=1   build without the embedded manifest (no themed controls / DPI
#              declaration; main.cpp still calls SetProcessDPIAware()).
# ---------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
  # MSYS2 and the standalone MinGW-W64 distributions both ship the unprefixed
  # g++ / windres. Override CXX and WINDRES if yours are named differently.
  CXX       := g++
  WINDRES   := windres
  # cmd.exe spelling of `rm -f`; the leading '-' on the clean recipe keeps a
  # mismatched shell from turning a tidy-up into a build failure.
  CLEAN_CMD := del /q
else
  # Linux: prefer the mingw-w64 cross toolchain, fall back to whatever is on
  # PATH (some distros install these under different names).
  ifneq (,$(shell command -v x86_64-w64-mingw32-g++ 2>/dev/null))
    CXX     := x86_64-w64-mingw32-g++
    WINDRES := x86_64-w64-mingw32-windres
  else
    CXX     := g++
    WINDRES := windres
  endif
  CLEAN_CMD := rm -f
endif

TARGET   := Notepad.exe
SRC      := main.cpp
RES      := app.rc

CXXFLAGS := -std=c++11 -O2 -Wall -Wextra
LDFLAGS  := -mwindows -static
# Explicit import libraries keep the link order correct and make the same
# file work with toolchains that do not pull these in by default.
LDLIBS   := -lcomdlg32 -lcomctl32 -lshell32 -lgdi32 -luser32 -ladvapi32

ifdef NO_RES
  OBJS := main.o
else
  OBJS := main.o app_res.o
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

main.o: $(SRC)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

app_res.o: $(RES) app.manifest
	$(WINDRES) -O coff -o $@ $(RES)

clean:
	-$(CLEAN_CMD) $(TARGET) main.o app_res.o

.PHONY: all clean
