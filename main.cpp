#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <wchar.h>
#include <string>
#include <vector>
#include <strsafe.h>

static decltype(&GetDeviceCaps) dyn_GetDeviceCaps = NULL;
static decltype(&CreateFontW) dyn_CreateFontW = NULL;
static decltype(&CreateFontIndirectW) dyn_CreateFontIndirectW = NULL;
static decltype(&SelectObject) dyn_SelectObject = NULL;
static decltype(&SetBkMode) dyn_SetBkMode = NULL;
static decltype(&SetTextColor) dyn_SetTextColor = NULL;
static decltype(&GetTextMetricsW) dyn_GetTextMetricsW = NULL;
static decltype(&StartDocW) dyn_StartDocW = NULL;
static decltype(&StartPage) dyn_StartPage = NULL;
static decltype(&EndPage) dyn_EndPage = NULL;
static decltype(&EndDoc) dyn_EndDoc = NULL;
static decltype(&DeleteObject) dyn_DeleteObject = NULL;
static decltype(&DeleteDC) dyn_DeleteDC = NULL;
static decltype(&CreatePen) dyn_CreatePen = NULL;
static decltype(&MoveToEx) dyn_MoveToEx = NULL;
static decltype(&LineTo) dyn_LineTo = NULL;
static decltype(&GetStockObject) dyn_GetStockObject = NULL;
static decltype(&GetOpenFileNameW) dyn_GetOpenFileNameW = NULL;
static decltype(&GetSaveFileNameW) dyn_GetSaveFileNameW = NULL;
static decltype(&FindTextW) dyn_FindTextW = NULL;
static decltype(&ReplaceTextW) dyn_ReplaceTextW = NULL;
static decltype(&PageSetupDlgW) dyn_PageSetupDlgW = NULL;
static decltype(&PrintDlgW) dyn_PrintDlgW = NULL;
static decltype(&CommDlgExtendedError) dyn_CommDlgExtendedError = NULL;
static decltype(&InitCommonControlsEx) dyn_InitCommonControlsEx = NULL;

#define LOAD_API(mod, name) \
    do { \
        dyn_##name = reinterpret_cast<decltype(dyn_##name)>(reinterpret_cast<void*>(GetProcAddress((mod), #name))); \
        if (!dyn_##name) goto api_fail; \
    } while (0)

/* Resolve only the genuine system copies of these DLLs. A plain LoadLibrary
   searches the application directory first, so a executable shipped next to a
   planted gdi32.dll / comctl32.dll would load the attacker's code instead
   (DLL search-order hijacking). */
#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#endif

static HMODULE LoadSystemLibrary(const wchar_t* name)
{
    HMODULE m = LoadLibraryExW(name, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (m) return m;

    /* Windows 7 without KB2533623 does not understand the flag above. Build an
       explicit %SystemRoot%\System32 path rather than falling back to the
       ambient search order. */
    wchar_t full[MAX_PATH];
    UINT n = GetSystemDirectoryW(full, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return NULL;
    size_t have = wcslen(full);
    size_t nlen = wcslen(name);
    if (have + nlen + 2 > (size_t)MAX_PATH) return NULL;
    full[have] = L'\\';
    full[have + 1] = L'\0';
    lstrcpyn(full + have + 1, name, (int)(MAX_PATH - have - 1));
    return LoadLibraryW(full);
}

static BOOL LoadDynamicApis()
{
    HMODULE gdi = LoadSystemLibrary(L"gdi32.dll");
    HMODULE cdl = LoadSystemLibrary(L"comdlg32.dll");
    HMODULE ccl = LoadSystemLibrary(L"comctl32.dll");
    if (!gdi || !cdl || !ccl) goto api_fail;
    LOAD_API(gdi, GetDeviceCaps);
    LOAD_API(gdi, CreateFontW);
    LOAD_API(gdi, CreateFontIndirectW);
    LOAD_API(gdi, SelectObject);
    LOAD_API(gdi, SetBkMode);
    LOAD_API(gdi, SetTextColor);
    LOAD_API(gdi, GetTextMetricsW);
    LOAD_API(gdi, StartDocW);
    LOAD_API(gdi, StartPage);
    LOAD_API(gdi, EndPage);
    LOAD_API(gdi, EndDoc);
    LOAD_API(gdi, DeleteObject);
    LOAD_API(gdi, DeleteDC);
    LOAD_API(gdi, CreatePen);
    LOAD_API(gdi, MoveToEx);
    LOAD_API(gdi, LineTo);
    LOAD_API(gdi, GetStockObject);
    LOAD_API(cdl, GetOpenFileNameW);
    LOAD_API(cdl, GetSaveFileNameW);
    LOAD_API(cdl, FindTextW);
    LOAD_API(cdl, ReplaceTextW);
    LOAD_API(cdl, PageSetupDlgW);
    LOAD_API(cdl, PrintDlgW);
    LOAD_API(cdl, CommDlgExtendedError);
    LOAD_API(ccl, InitCommonControlsEx);
    return TRUE;

api_fail:
    /* Never leak the module handles when a single export is missing. */
    if (gdi) FreeLibrary(gdi);
    if (cdl) FreeLibrary(cdl);
    if (ccl) FreeLibrary(ccl);
    return FALSE;
}

#define GetDeviceCaps dyn_GetDeviceCaps
#define CreateFontW dyn_CreateFontW
#define CreateFontIndirectW dyn_CreateFontIndirectW
#define SelectObject dyn_SelectObject
#define SetBkMode dyn_SetBkMode
#define SetTextColor dyn_SetTextColor
#define GetTextMetricsW dyn_GetTextMetricsW
#define StartDocW dyn_StartDocW
#define StartPage dyn_StartPage
#define EndPage dyn_EndPage
#define EndDoc dyn_EndDoc
#define DeleteObject dyn_DeleteObject
#define DeleteDC dyn_DeleteDC
#define CreatePen dyn_CreatePen
#define MoveToEx dyn_MoveToEx
#define LineTo dyn_LineTo
#define GetStockObject dyn_GetStockObject
#define GetOpenFileNameW dyn_GetOpenFileNameW
#define GetSaveFileNameW dyn_GetSaveFileNameW
#define FindTextW dyn_FindTextW
#define ReplaceTextW dyn_ReplaceTextW
#define PageSetupDlgW dyn_PageSetupDlgW
#define PrintDlgW dyn_PrintDlgW
#define CommDlgExtendedError dyn_CommDlgExtendedError
#define InitCommonControlsEx dyn_InitCommonControlsEx

#define IDC_TAB_CTRL        3000
#define IDC_STATUS          3001
#define IDC_EDIT            4000
#define IDC_GOTO_EDIT       100

#define IDM_FILE_NEW        3101
#define IDM_FILE_OPEN       3102
#define IDM_FILE_SAVE       3103
#define IDM_FILE_SAVEAS     3104
#define IDM_FILE_CLOSETAB   3105
#define IDM_FILE_PAGESETUP  3106
#define IDM_FILE_PRINT      3107
#define IDM_FILE_EXIT       3108

#define IDM_EDIT_UNDO       3201
#define IDM_EDIT_CUT        3202
#define IDM_EDIT_COPY       3203
#define IDM_EDIT_PASTE      3204
#define IDM_EDIT_DELETE     3205
#define IDM_EDIT_FIND       3206
#define IDM_EDIT_FINDNEXT   3207
#define IDM_EDIT_FINDPREV   3208
#define IDM_EDIT_REPLACE    3209
#define IDM_EDIT_GOTO       3210
#define IDM_EDIT_SELECTALL  3211
#define IDM_EDIT_TIME       3212

#define IDM_ENC_ANSI        3301
#define IDM_ENC_UTF8        3302
#define IDM_ENC_UTF8BOM     3303
#define IDM_ENC_UTF16LE     3304
#define IDM_ENC_UTF16BE     3305

#define IDM_VIEW_WRAP       3401
#define IDM_VIEW_STATUSBAR  3402
#define IDM_TAB_NEXT        3403
#define IDM_TAB_PREV        3404

#define IDM_HELP_ABOUT      3501

/* Posted to ourselves after a layout pass: the status bar positions itself
   from inside its own WM_SIZE handler, so a repaint requested during the
   parent's WM_SIZE is swallowed. Doing it on a later message works. */
#define WM_APP_REDRAW_STATUS (WM_APP + 1)

#define MAX_PATH_BUF        1024
#define FIND_BUFFER_SIZE    256
#define MAX_FILE_BYTES      (100u * 1024u * 1024u)
#define TAB_WIDTH           180
#define TAB_HEIGHT          28

enum { ENC_ANSI = 0, ENC_UTF8, ENC_UTF8_BOM, ENC_UTF16LE, ENC_UTF16BE };

struct Tab {
    HWND hwndEdit;
    wchar_t path[MAX_PATH_BUF];
    BOOL modified;
    int encoding;
};

static const size_t NPOS = (size_t)-1;

static HINSTANCE g_hInst = NULL;
static HWND g_hwndMain = NULL;
static HWND g_hwndTab = NULL;
static HWND g_hwndStatus = NULL;
static HFONT g_hUIFont = NULL;
static HFONT g_hEditFont = NULL;
static HACCEL g_hAccel = NULL;
static HMENU g_hEditMenu = NULL;
static HMENU g_hViewMenu = NULL;
static HMENU g_hEncMenu = NULL;
static std::vector<Tab> g_tabs;
static int g_activeTab = -1;
static int g_suppressChange = 0;
static BOOL g_wrap = FALSE;
static BOOL g_showStatus = TRUE;
static int g_dpi = 96;
static DWORD g_lastCloseTick = 0;
static int g_lastClosePtX = -100000;
static int g_lastClosePtY = -100000;

static UINT g_findMsg = 0;
static FINDREPLACE g_frFind;
static FINDREPLACE g_frReplace;
static HWND g_hFindDlg = NULL;
static HWND g_hReplaceDlg = NULL;
static wchar_t g_findWhat[FIND_BUFFER_SIZE] = L"";
static wchar_t g_replaceWith[FIND_BUFFER_SIZE] = L"";
static DWORD g_findFlags = FR_DOWN;
static PAGESETUPDLG g_psd;
static int g_gotoValue = 1;
static DWORD g_dlgBuf[256];

static void SelectTab(int idx);
static void UpdateTabTitle(int idx);
static void UpdateWindowTitle();
static void UpdateStatusBar();
static void UpdateMenuChecks();
static void LayoutChildren();
static BOOL SaveTab(int idx, BOOL forceDialog);
static int AddNewTab();
static void RemoveTab(int idx);
static void ShowFindDialog(BOOL replace);

static int Max(int a, int b)
{
    return a > b ? a : b;
}

/* All hard-coded pixel constants in this file are authored at 96 DPI and
   scaled through this helper so the layout stays correct on high-DPI
   displays (Windows 7 through Windows 11). */
static int Scale(int v)
{
    if (g_dpi <= 0) return v;
    return MulDiv(v, g_dpi, 96);
}

static void GetEditString(HWND edit, std::wstring& s)
{
    int len = GetWindowTextLength(edit);
    s.assign((size_t)len + 1, L'\0');
    int got = GetWindowText(edit, &s[0], len + 1);
    if (got < 0) got = 0;
    s.resize((size_t)got);
}

static HWND ActiveEdit()
{
    if (g_activeTab < 0 || g_activeTab >= (int)g_tabs.size()) return NULL;
    return g_tabs[(size_t)g_activeTab].hwndEdit;
}

static void BuildTabName(int idx, wchar_t* out, int cap, BOOL star)
{
    if (!out || cap < 2) return;
    const wchar_t* path = g_tabs[(size_t)idx].path;
    const wchar_t* name = L"Untitled";
    if (path[0]) {
        const wchar_t* a = wcsrchr(path, L'\\');
        const wchar_t* b = wcsrchr(path, L'/');
        const wchar_t* s = a;
        if (b && (!s || b > s)) s = b;
        name = s ? s + 1 : path;
    }
    lstrcpyn(out, name, cap - 1);
    if (star && g_tabs[(size_t)idx].modified) {
        /* Append the dirty marker explicitly and only when it is known to fit;
           lstrcat() would trust the destination blindly. */
        int len = lstrlen(out);
        if (len + 2 <= cap) {
            out[len] = L'*';
            out[len + 1] = L'\0';
        }
    }
}

static void UpdateWindowTitle()
{
    if (!g_hwndMain) return;
    if (g_activeTab < 0 || g_activeTab >= (int)g_tabs.size()) {
        SetWindowText(g_hwndMain, L"Notepad");
        return;
    }
    wchar_t name[MAX_PATH_BUF + 4];
    BuildTabName(g_activeTab, name, MAX_PATH_BUF + 4, FALSE);
    std::wstring t;
    if (g_tabs[(size_t)g_activeTab].modified) t += L"*";
    t += name;
    t += L" - Notepad";
    SetWindowText(g_hwndMain, t.c_str());
}

static void UpdateTabTitle(int idx)
{
    if (idx < 0 || idx >= (int)g_tabs.size() || !g_hwndTab) return;
    wchar_t name[MAX_PATH_BUF + 4];
    BuildTabName(idx, name, MAX_PATH_BUF + 4, TRUE);
    TCITEM tie;
    ZeroMemory(&tie, sizeof(tie));
    tie.mask = TCIF_TEXT;
    tie.pszText = name;
    SendMessage(g_hwndTab, TCM_SETITEM, (WPARAM)idx, (LPARAM)&tie);
    InvalidateRect(g_hwndTab, NULL, FALSE);
    if (idx == g_activeTab) UpdateWindowTitle();
}

static void SetTabModified(int idx, BOOL f)
{
    if (idx < 0 || idx >= (int)g_tabs.size()) return;
    if (g_tabs[(size_t)idx].modified != f) {
        g_tabs[(size_t)idx].modified = f;
        UpdateTabTitle(idx);
    }
}

static const wchar_t* EncodingName(int enc)
{
    switch (enc) {
    case ENC_ANSI: return L"ANSI";
    case ENC_UTF8: return L"UTF-8";
    case ENC_UTF8_BOM: return L"UTF-8 with BOM";
    case ENC_UTF16LE: return L"UTF-16 LE";
    case ENC_UTF16BE: return L"UTF-16 BE";
    }
    return L"UTF-8";
}

static void SetStatusPart(int part, const wchar_t* text)
{
    static wchar_t cache[4][96];
    if (!g_hwndStatus || part < 0 || part > 3) return;
    if (lstrcmp(cache[part], text) == 0) return;
    lstrcpyn(cache[part], text, 96);
    SendMessage(g_hwndStatus, SB_SETTEXT, (WPARAM)part, (LPARAM)text);
}

static void UpdateStatusBar()
{
    HWND e = ActiveEdit();
    if (!e || !g_hwndStatus) return;
    DWORD s = 0, en = 0;
    SendMessage(e, EM_GETSEL, (WPARAM)&s, (LPARAM)&en);
    int line = (int)SendMessage(e, EM_LINEFROMCHAR, (WPARAM)s, 0);
    int ls = (int)SendMessage(e, EM_LINEINDEX, (WPARAM)line, 0);
    if (ls < 0) ls = 0;
    wchar_t buf[96];
    StringCchPrintfW(buf, 96, L"Ln %d, Col %d", line + 1, (int)s - ls + 1);
    SetStatusPart(0, buf);
    StringCchPrintfW(buf, 96, L"%d characters", GetWindowTextLength(e));
    SetStatusPart(1, buf);
    SetStatusPart(2, L"Windows (CRLF)");
    SetStatusPart(3, EncodingName(g_tabs[(size_t)g_activeTab].encoding));
}

static void UpdateMenuChecks()
{
    if (g_hViewMenu) {
        CheckMenuItem(g_hViewMenu, IDM_VIEW_WRAP, MF_BYCOMMAND | (g_wrap ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(g_hViewMenu, IDM_VIEW_STATUSBAR, MF_BYCOMMAND | (g_showStatus ? MF_CHECKED : MF_UNCHECKED));
    }
    if (g_hEncMenu && g_activeTab >= 0 && g_activeTab < (int)g_tabs.size()) {
        CheckMenuRadioItem(g_hEncMenu, IDM_ENC_ANSI, IDM_ENC_UTF16BE,
                           (UINT)(IDM_ENC_ANSI + g_tabs[(size_t)g_activeTab].encoding), MF_BYCOMMAND);
    }
}

static HFONT CreateEditFont()
{
    return CreateFont(-MulDiv(11, g_dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
}

/* The stock DEFAULT_GUI_FONT is baked at 96 DPI. Once the process is DPI
   aware we must build our own UI font, otherwise the tab strip and the
   status bar render far too small on scaled displays. The caller owns the
   returned handle. */
static HFONT CreateUIFont()
{
    return CreateFont(-MulDiv(9, g_dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"MS Shell Dlg");
}

static HWND CreateEditControl()
{
    DWORD style = WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_WANTRETURN;
    if (!g_wrap) style |= WS_HSCROLL | ES_AUTOHSCROLL;
    HWND h = CreateWindowEx(0, L"EDIT", L"", style, 0, 0, 100, 100,
                            g_hwndMain, (HMENU)(INT_PTR)IDC_EDIT, g_hInst, NULL);
    if (!h) return NULL;
    SendMessage(h, WM_SETFONT, (WPARAM)g_hEditFont, FALSE);
    SendMessage(h, EM_SETLIMITTEXT, 0, 0);
    return h;
}

static void LayoutChildren()
{
    if (!g_hwndMain || !g_hwndTab || !g_hwndStatus) return;
    RECT rc;
    GetClientRect(g_hwndMain, &rc);
    int sh = 0;
    if (g_showStatus) {
        SendMessage(g_hwndStatus, WM_SIZE, 0, 0);
        RECT sr;
        GetWindowRect(g_hwndStatus, &sr);
        sh = sr.bottom - sr.top;
        int parts[4];
        parts[3] = -1;
        parts[2] = Max((int)rc.right - Scale(120), Scale(180));
        parts[1] = parts[2] - Scale(110);
        parts[0] = parts[1] - Scale(120);
        if (parts[0] < Scale(20)) parts[0] = Scale(20);
        if (parts[1] < parts[0] + Scale(10)) parts[1] = parts[0] + Scale(10);
        if (parts[2] < parts[1] + Scale(10)) parts[2] = parts[1] + Scale(10);
        SendMessage(g_hwndStatus, SB_SETPARTS, 4, (LPARAM)parts);
    }
    /* Available area above the status bar. */
    int areaH = Max((int)rc.bottom - sh, 0);
    if (rc.right < 0) rc.right = 0;

    /* The tab control must cover the strip only. The old code sized it to the
       whole client area, so the tab control and the edit control overlapped
       completely. With WS_CLIPSIBLINGS on both siblings, each one is clipped
       out of the other's rectangle - and because the edit lived entirely
       inside the tab's rectangle, the edit was never painted at all and the
       document appeared to vanish. */
    MoveWindow(g_hwndTab, 0, 0, rc.right, areaH, TRUE);
    RECT d;
    d.left = 0;
    d.top = 0;
    d.right = rc.right;
    d.bottom = areaH;
    SendMessage(g_hwndTab, TCM_ADJUSTRECT, FALSE, (LPARAM)&d);
    int strip = Max((int)d.top, 0);
    if (strip > areaH) strip = areaH;
    MoveWindow(g_hwndTab, 0, 0, rc.right, strip, TRUE);

    int w = Max((int)rc.right, 0);
    int h = Max(areaH - strip, 0);
    for (size_t i = 0; i < g_tabs.size(); i++) {
        if (g_tabs[i].hwndEdit) MoveWindow(g_tabs[i].hwndEdit, 0, strip, w, h, TRUE);
    }

    /* Place the status bar explicitly instead of relying solely on the control
       re-docking itself from its WM_SIZE handler, and keep it above the
       editors so it can never end up behind one. */
    if (g_hwndStatus && g_showStatus) {
        /* A zero-height measurement would otherwise let the editor cover the
           strip entirely. */
        if (sh <= 0) sh = Scale(20);
        MoveWindow(g_hwndStatus, 0, areaH, rc.right, sh, TRUE);
        SetWindowPos(g_hwndStatus, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        /* Ask for a repaint once the current layout pass has finished. */
        PostMessage(g_hwndMain, WM_APP_REDRAW_STATUS, 0, 0);
    }
    InvalidateRect(g_hwndMain, NULL, TRUE);
}

static void SelectTab(int idx)
{
    if (idx < 0 || idx >= (int)g_tabs.size()) return;
    g_activeTab = idx;
    for (size_t i = 0; i < g_tabs.size(); i++) {
        ShowWindow(g_tabs[i].hwndEdit, (int)i == idx ? SW_SHOW : SW_HIDE);
    }
    SendMessage(g_hwndTab, TCM_SETCURSEL, (WPARAM)idx, 0);
    SetFocus(g_tabs[(size_t)idx].hwndEdit);
    UpdateWindowTitle();
    UpdateMenuChecks();
    UpdateStatusBar();
}

static int AddNewTab()
{
    HWND e = CreateEditControl();
    if (!e) return -1;
    Tab t;
    ZeroMemory(&t, sizeof(t));
    t.hwndEdit = e;
    t.encoding = ENC_UTF8;
    g_tabs.push_back(t);
    int idx = (int)g_tabs.size() - 1;
    TCITEM tie;
    ZeroMemory(&tie, sizeof(tie));
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)L"Untitled";
    SendMessage(g_hwndTab, TCM_INSERTITEM, (WPARAM)idx, (LPARAM)&tie);
    LayoutChildren();
    SelectTab(idx);
    return idx;
}

static void RemoveTab(int idx)
{
    if (idx < 0 || idx >= (int)g_tabs.size()) return;
    HWND e = g_tabs[(size_t)idx].hwndEdit;
    BOOL wasActive = (idx == g_activeTab);
    int prev = g_activeTab;
    g_tabs.erase(g_tabs.begin() + idx);
    if (wasActive) g_activeTab = -1;
    SendMessage(g_hwndTab, TCM_DELETEITEM, (WPARAM)idx, 0);
    DestroyWindow(e);
    if (g_tabs.empty()) {
        g_activeTab = -1;
        UpdateWindowTitle();
        /* The status bar still describes the tab we just destroyed. */
        SetStatusPart(0, L"");
        SetStatusPart(1, L"");
        SetStatusPart(2, L"");
        SetStatusPart(3, L"");
        return;
    }
    if (wasActive) {
        int n = idx < (int)g_tabs.size() ? idx : (int)g_tabs.size() - 1;
        SelectTab(n);
    } else {
        g_activeTab = idx < prev ? prev - 1 : prev;
        SendMessage(g_hwndTab, TCM_SETCURSEL, (WPARAM)g_activeTab, 0);
        UpdateWindowTitle();
    }
}

static BOOL MbToWide(const char* s, int len, UINT cp, DWORD flags, std::wstring& out)
{
    out.clear();
    if (len <= 0) return TRUE;
    int n = MultiByteToWideChar(cp, flags, s, len, NULL, 0);
    if (n <= 0) return FALSE;
    out.assign((size_t)n, L'\0');
    MultiByteToWideChar(cp, flags, s, len, &out[0], n);
    return TRUE;
}

/* Returns 0 when the buffer is not UTF-16, 1 for little endian and 2 for big
   endian. This catches the very common "UTF-16 without BOM" layout that the
   old code decoded as mojibake. */
static int DetectUtf16NoBom(const unsigned char* d, size_t n)
{
    if (n < 4) return 0;
    size_t sample = n > 4096 ? 4096 : n;
    if (sample & 1) sample--;
    size_t pairs = sample / 2;
    if (pairs < 2) return 0;
    size_t zeroEven = 0, zeroOdd = 0;
    for (size_t i = 0; i + 1 < sample; i += 2) {
        if (d[i] == 0) zeroEven++;
        if (d[i + 1] == 0) zeroOdd++;
    }
    /* Little endian stores the zero high byte at the ODD index, big endian
       at the EVEN index - getting that backwards swaps every character. */
    if (zeroOdd >= (pairs * 3) / 4 && zeroEven <= pairs / 8) return 1;
    if (zeroEven >= (pairs * 3) / 4 && zeroOdd <= pairs / 8) return 2;
    return 0;
}

static void DecodeUtf16(const unsigned char* d, size_t start, size_t n, BOOL be, std::wstring& out)
{
    size_t cnt = start < n ? (n - start) / 2 : 0;
    out.assign(cnt, L'\0');
    for (size_t i = 0; i < cnt; i++) {
        unsigned int a = d[start + i * 2];
        unsigned int c = d[start + i * 2 + 1];
        out[i] = be ? (wchar_t)((a << 8) | c) : (wchar_t)((c << 8) | a);
    }
}

static void DecodeFileBytes(const std::vector<unsigned char>& b, std::wstring& out, int& enc)
{
    size_t n = b.size();
    out.clear();
    enc = ENC_UTF8;
    if (n == 0) return;
    const unsigned char* d = b.data();
    if (n >= 3 && d[0] == 0xEF && d[1] == 0xBB && d[2] == 0xBF) {
        enc = ENC_UTF8_BOM;
        if (!MbToWide((const char*)d + 3, (int)(n - 3), CP_UTF8, 0, out)) {
            enc = ENC_ANSI;
            MbToWide((const char*)d + 3, (int)(n - 3), CP_ACP, 0, out);
        }
        return;
    }
    if (n >= 2 && ((d[0] == 0xFF && d[1] == 0xFE) || (d[0] == 0xFE && d[1] == 0xFF))) {
        BOOL be = (d[0] == 0xFE);
        enc = be ? ENC_UTF16BE : ENC_UTF16LE;
        DecodeUtf16(d, 2, n, be, out);
        return;
    }
    /* UTF-16 saved without a byte order mark: without this check the file was
       decoded as UTF-8/ANSI and filled the editor with interleaved NULs. */
    int bomless = DetectUtf16NoBom(d, n);
    if (bomless) {
        enc = (bomless == 2) ? ENC_UTF16BE : ENC_UTF16LE;
        DecodeUtf16(d, 0, n, bomless == 2, out);
        return;
    }
    if (MbToWide((const char*)d, (int)n, CP_UTF8, MB_ERR_INVALID_CHARS, out)) {
        enc = ENC_UTF8;
        return;
    }
    enc = ENC_ANSI;
    MbToWide((const char*)d, (int)n, CP_ACP, 0, out);
}

static void NormalizeNewlines(const std::wstring& in, std::wstring& out)
{
    out.clear();
    out.reserve(in.size() + in.size() / 32 + 16);
    size_t n = in.size();
    for (size_t i = 0; i < n; i++) {
        wchar_t c = in[i];
        if (c == L'\r') {
            out += L"\r\n";
            if (i + 1 < n && in[i + 1] == L'\n') i++;
        } else if (c == L'\n') {
            out += L"\r\n";
        } else if (c == L'\0') {
            out += L' ';
        } else {
            out += c;
        }
    }
}

static BOOL EncodeText(const std::wstring& w, int& enc, std::vector<unsigned char>& out)
{
    out.clear();
    if (enc == ENC_UTF16LE || enc == ENC_UTF16BE) {
        BOOL be = (enc == ENC_UTF16BE);
        out.push_back(be ? 0xFE : 0xFF);
        out.push_back(be ? 0xFF : 0xFE);
        for (size_t i = 0; i < w.size(); i++) {
            unsigned char lo = (unsigned char)(w[i] & 0xFF);
            unsigned char hi = (unsigned char)((w[i] >> 8) & 0xFF);
            if (be) {
                out.push_back(hi);
                out.push_back(lo);
            } else {
                out.push_back(lo);
                out.push_back(hi);
            }
        }
        return TRUE;
    }
    if (enc == ENC_ANSI) {
        if (w.empty()) return TRUE;
        BOOL used = FALSE;
        int n = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, w.c_str(), (int)w.size(), NULL, 0, NULL, &used);
        if (n > 0 && !used) {
            out.assign((size_t)n, 0);
            WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, w.c_str(), (int)w.size(), (LPSTR)&out[0], n, NULL, &used);
            return TRUE;
        }
        enc = ENC_UTF8;
    }
    if (enc == ENC_UTF8_BOM) {
        out.push_back(0xEF);
        out.push_back(0xBB);
        out.push_back(0xBF);
    }
    if (!w.empty()) {
        int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
        if (n <= 0) return FALSE;
        size_t base = out.size();
        out.resize(base + (size_t)n);
        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), (LPSTR)&out[base], n, NULL, NULL);
    }
    return TRUE;
}

static BOOL ReadWholeFile(const wchar_t* path, std::vector<unsigned char>& data, BOOL& tooLarge)
{
    tooLarge = FALSE;
    HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return FALSE;
    LARGE_INTEGER sz;
    if (!GetFileSizeEx(h, &sz)) {
        CloseHandle(h);
        return FALSE;
    }
    if (sz.QuadPart > (LONGLONG)MAX_FILE_BYTES) {
        tooLarge = TRUE;
        CloseHandle(h);
        return FALSE;
    }
    data.assign((size_t)sz.QuadPart, 0);
    size_t total = 0;
    while (total < data.size()) {
        size_t remain = data.size() - total;
        DWORD chunk = (DWORD)(remain > 0x1000000u ? 0x1000000u : remain);
        DWORD got = 0;
        if (!ReadFile(h, &data[total], chunk, &got, NULL) || got == 0) break;
        total += got;
    }
    CloseHandle(h);
    data.resize(total);
    return TRUE;
}

static BOOL WriteWholeFile(const wchar_t* path, const std::vector<unsigned char>& data)
{
    HANDLE h = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return FALSE;
    size_t total = 0;
    BOOL ok = TRUE;
    while (total < data.size()) {
        size_t remain = data.size() - total;
        DWORD chunk = (DWORD)(remain > 0x1000000u ? 0x1000000u : remain);
        DWORD wrote = 0;
        if (!WriteFile(h, &data[total], chunk, &wrote, NULL) || wrote == 0) {
            ok = FALSE;
            break;
        }
        total += wrote;
    }
    CloseHandle(h);
    return ok;
}

static BOOL LoadFileIntoTabImpl(int idx, const wchar_t* path)
{
    std::vector<unsigned char> bytes;
    BOOL tooLarge = FALSE;
    if (!ReadWholeFile(path, bytes, tooLarge)) {
        std::wstring m = tooLarge ? L"The file is too large to open:\n" : L"Cannot open the file:\n";
        m += path;
        MessageBox(g_hwndMain, m.c_str(), L"Notepad", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    std::wstring raw, text;
    int enc = ENC_UTF8;
    DecodeFileBytes(bytes, raw, enc);
    NormalizeNewlines(raw, text);
    HWND e = g_tabs[(size_t)idx].hwndEdit;
    g_suppressChange++;
    SetWindowText(e, text.c_str());
    g_suppressChange--;
    SendMessage(e, EM_SETSEL, 0, 0);
    SendMessage(e, EM_SCROLLCARET, 0, 0);
    SendMessage(e, EM_EMPTYUNDOBUFFER, 0, 0);
    SendMessage(e, EM_SETMODIFY, FALSE, 0);
    lstrcpyn(g_tabs[(size_t)idx].path, path, MAX_PATH_BUF);
    g_tabs[(size_t)idx].encoding = enc;
    SetTabModified(idx, FALSE);
    UpdateTabTitle(idx);
    if (idx == g_activeTab) {
        UpdateMenuChecks();
        UpdateStatusBar();
    }
    return TRUE;
}

/* Reading, decoding and SetWindowText all allocate. A std::bad_alloc used to
   escape straight to std::terminate() and took the whole process - and every
   unsaved tab - down with it. */
static BOOL LoadFileIntoTab(int idx, const wchar_t* path)
{
    try {
        return LoadFileIntoTabImpl(idx, path);
    } catch (...) {
        MessageBox(g_hwndMain, L"Not enough memory to open this file.", L"Notepad",
                   MB_OK | MB_ICONERROR);
        return FALSE;
    }
}

static BOOL ShowOpenDialog(HWND owner, wchar_t* path, int cap)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    path[0] = L'\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"Text Documents (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = path;
    ofn.nMaxFile = (DWORD)cap;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_EXPLORER;
    return GetOpenFileName(&ofn);
}

static BOOL ShowSaveDialog(HWND owner, wchar_t* path, int cap)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"Text Documents (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = path;
    ofn.nMaxFile = (DWORD)cap;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_EXPLORER;
    return GetSaveFileName(&ofn);
}

static BOOL SaveTabImpl(int idx, BOOL forceDialog)
{
    if (idx < 0 || idx >= (int)g_tabs.size()) return FALSE;
    wchar_t path[MAX_PATH_BUF];
    lstrcpyn(path, g_tabs[(size_t)idx].path, MAX_PATH_BUF);
    if (forceDialog || !path[0]) {
        if (!ShowSaveDialog(g_hwndMain, path, MAX_PATH_BUF)) return FALSE;
    }
    HWND e = g_tabs[(size_t)idx].hwndEdit;
    std::wstring text;
    GetEditString(e, text);
    int enc = g_tabs[(size_t)idx].encoding;
    std::vector<unsigned char> bytes;
    if (!EncodeText(text, enc, bytes) || !WriteWholeFile(path, bytes)) {
        std::wstring m = L"Cannot save the file:\n";
        m += path;
        MessageBox(g_hwndMain, m.c_str(), L"Notepad", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    lstrcpyn(g_tabs[(size_t)idx].path, path, MAX_PATH_BUF);
    g_tabs[(size_t)idx].encoding = enc;
    SendMessage(e, EM_SETMODIFY, FALSE, 0);
    SetTabModified(idx, FALSE);
    UpdateTabTitle(idx);
    if (idx == g_activeTab) {
        UpdateMenuChecks();
        UpdateStatusBar();
    }
    return TRUE;
}

static BOOL SaveTab(int idx, BOOL forceDialog)
{
    try {
        return SaveTabImpl(idx, forceDialog);
    } catch (...) {
        MessageBox(g_hwndMain, L"Not enough memory to save this file.", L"Notepad",
                   MB_OK | MB_ICONERROR);
        return FALSE;
    }
}

static BOOL ConfirmSave(int idx)
{
    if (idx < 0 || idx >= (int)g_tabs.size()) return TRUE;
    if (!g_tabs[(size_t)idx].modified) return TRUE;
    wchar_t name[MAX_PATH_BUF + 4];
    BuildTabName(idx, name, MAX_PATH_BUF + 4, FALSE);
    std::wstring m = L"Do you want to save changes to ";
    m += name;
    m += L"?";
    int r = MessageBox(g_hwndMain, m.c_str(), L"Notepad", MB_YESNOCANCEL | MB_ICONWARNING);
    if (r == IDYES) return SaveTab(idx, FALSE);
    if (r == IDNO) return TRUE;
    return FALSE;
}

static void CloseTab(int idx)
{
    if (idx < 0 || idx >= (int)g_tabs.size()) return;
    if (g_tabs[(size_t)idx].modified) {
        SelectTab(idx);
        if (!ConfirmSave(idx)) return;
    }
    if (g_tabs.size() == 1) {
        DestroyWindow(g_hwndMain);
        return;
    }
    RemoveTab(idx);
}

static BOOL OpenPath(const wchar_t* path)
{
    wchar_t full[MAX_PATH_BUF];
    DWORD n = GetFullPathName(path, MAX_PATH_BUF, full, NULL);
    if (n == 0 || n >= MAX_PATH_BUF) lstrcpyn(full, path, MAX_PATH_BUF);
    for (size_t i = 0; i < g_tabs.size(); i++) {
        if (lstrcmpi(g_tabs[i].path, full) == 0) {
            SelectTab((int)i);
            return TRUE;
        }
    }
    BOOL reuse = FALSE;
    int idx = 0;
    if (g_tabs.size() == 1 && !g_tabs[0].path[0] && !g_tabs[0].modified &&
        GetWindowTextLength(g_tabs[0].hwndEdit) == 0) {
        reuse = TRUE;
        idx = 0;
    } else {
        idx = AddNewTab();
        if (idx < 0) return FALSE;
    }
    if (!LoadFileIntoTab(idx, full)) {
        if (!reuse) RemoveTab(idx);
        return FALSE;
    }
    SelectTab(idx);
    return TRUE;
}

static void ApplyWrapToAllTabs()
{
    for (size_t i = 0; i < g_tabs.size(); i++) {
        HWND oldE = g_tabs[i].hwndEdit;
        std::wstring text;
        GetEditString(oldE, text);
        DWORD s = 0, en = 0;
        SendMessage(oldE, EM_GETSEL, (WPARAM)&s, (LPARAM)&en);
        HWND ne = CreateEditControl();
        if (!ne) continue;
        g_suppressChange++;
        SetWindowText(ne, text.c_str());
        g_suppressChange--;
        g_tabs[i].hwndEdit = ne;
        DestroyWindow(oldE);
        LayoutChildren();
        if ((int)i == g_activeTab) ShowWindow(ne, SW_SHOW);
        SendMessage(ne, EM_SETSEL, (WPARAM)s, (LPARAM)en);
        SendMessage(ne, EM_SCROLLCARET, 0, 0);
        /* Recreating the control resets its modify flag (and its undo
           buffer); keep the control in sync with the tab's dirty state so a
           later save/close decision is not wrong. */
        SendMessage(ne, EM_SETMODIFY, g_tabs[i].modified ? TRUE : FALSE, 0);
    }
    LayoutChildren();
    if (g_activeTab >= 0) SelectTab(g_activeTab);
}

static BOOL IsWordChar(wchar_t c)
{
    return c == L'_' || IsCharAlphaNumeric(c);
}

static BOOL WordBoundaryOk(const std::wstring& t, size_t pos, size_t len)
{
    if (pos > 0 && IsWordChar(t[pos - 1])) return FALSE;
    if (pos + len < t.size() && IsWordChar(t[pos + len])) return FALSE;
    return TRUE;
}

static void PrepareHaystack(const std::wstring& text, const wchar_t* what, BOOL matchCase,
                            std::wstring& hay, std::wstring& needle)
{
    hay = text;
    needle = what;
    if (!matchCase) {
        if (!hay.empty()) CharLowerBuff(&hay[0], (DWORD)hay.size());
        if (!needle.empty()) CharLowerBuff(&needle[0], (DWORD)needle.size());
    }
}

static void ShowNotFound(const wchar_t* what, HWND owner)
{
    std::wstring m = L"Cannot find \"";
    m += what;
    m += L"\"";
    MessageBox(owner ? owner : g_hwndMain, m.c_str(), L"Notepad", MB_OK | MB_ICONINFORMATION);
}

static BOOL SearchEdit(HWND edit, const wchar_t* what, BOOL down, BOOL matchCase, BOOL wholeWord, HWND msgOwner)
{
    if (!edit || !what[0]) return FALSE;
    std::wstring text, hay, needle;
    GetEditString(edit, text);
    PrepareHaystack(text, what, matchCase, hay, needle);
    DWORD s = 0, e = 0;
    SendMessage(edit, EM_GETSEL, (WPARAM)&s, (LPARAM)&e);
    size_t flen = needle.size();
    size_t pos = NPOS;
    if (down) {
        size_t from = e;
        /* std::wstring::find throws if pos > size(); clamp defensively. */
        if (from > hay.size()) from = hay.size();
        while (from <= hay.size()) {
            size_t p = hay.find(needle, from);
            if (p == NPOS) break;
            if (!wholeWord || WordBoundaryOk(hay, p, flen)) {
                pos = p;
                break;
            }
            from = p + 1;
        }
    } else if (s >= flen) {
        size_t from = s - flen;
        for (;;) {
            size_t p = hay.rfind(needle, from);
            if (p == NPOS) break;
            if (!wholeWord || WordBoundaryOk(hay, p, flen)) {
                pos = p;
                break;
            }
            if (p == 0) break;
            from = p - 1;
        }
    }
    if (pos != NPOS) {
        SendMessage(edit, EM_SETSEL, (WPARAM)pos, (LPARAM)(pos + flen));
        SendMessage(edit, EM_SCROLLCARET, 0, 0);
        return TRUE;
    }
    ShowNotFound(what, msgOwner);
    return FALSE;
}

static void ReplaceAllInEdit(HWND edit, const wchar_t* what, const wchar_t* replaceWith,
                             BOOL matchCase, BOOL wholeWord, HWND msgOwner)
{
    if (!edit || !what[0]) return;
    if (!replaceWith) replaceWith = L"";
    std::wstring text, hay, needle, result;
    GetEditString(edit, text);
    PrepareHaystack(text, what, matchCase, hay, needle);
    size_t flen = needle.size();
    size_t wlen = (size_t)lstrlen(replaceWith);
    size_t from = 0, last = 0;
    int count = 0;
    while (from <= hay.size()) {
        size_t p = hay.find(needle, from);
        if (p == NPOS) break;
        if (wholeWord && !WordBoundaryOk(hay, p, flen)) {
            from = p + 1;
            continue;
        }
        result.append(text, last, p - last);
        result.append(replaceWith, wlen);
        last = p + flen;
        from = last;
        count++;
    }
    if (count == 0) {
        ShowNotFound(what, msgOwner);
        return;
    }
    result.append(text, last, NPOS);
    SendMessage(edit, EM_SETSEL, 0, (LPARAM)-1);
    SendMessage(edit, EM_REPLACESEL, TRUE, (LPARAM)result.c_str());
    SendMessage(edit, EM_SETSEL, 0, 0);
    SendMessage(edit, EM_SCROLLCARET, 0, 0);
}

static void ReplaceCurrent(HWND edit, LPFINDREPLACE fr, HWND dlg)
{
    DWORD s = 0, e = 0;
    SendMessage(edit, EM_GETSEL, (WPARAM)&s, (LPARAM)&e);
    BOOL matchCase = (fr->Flags & FR_MATCHCASE) != 0;
    BOOL wholeWord = (fr->Flags & FR_WHOLEWORD) != 0;
    if (e > s) {
        std::wstring text;
        GetEditString(edit, text);
        if (e <= text.size()) {
            std::wstring sel = text.substr(s, e - s);
            BOOL eq = matchCase ? (lstrcmp(sel.c_str(), fr->lpstrFindWhat) == 0)
                                : (lstrcmpi(sel.c_str(), fr->lpstrFindWhat) == 0);
            if (eq) SendMessage(edit, EM_REPLACESEL, TRUE, (LPARAM)fr->lpstrReplaceWith);
        }
    }
    SearchEdit(edit, fr->lpstrFindWhat, TRUE, matchCase, wholeWord, dlg);
}

static void HandleFindMessage(LPFINDREPLACE fr)
{
    if (fr->Flags & FR_DIALOGTERM) {
        if (fr == &g_frFind) g_hFindDlg = NULL;
        else g_hReplaceDlg = NULL;
        return;
    }
    BOOL fromReplace = (fr == &g_frReplace);
    g_findFlags = fr->Flags & (FR_MATCHCASE | FR_WHOLEWORD);
    if (fromReplace || (fr->Flags & FR_DOWN)) g_findFlags |= FR_DOWN;
    HWND edit = ActiveEdit();
    if (!edit) return;
    HWND dlg = fromReplace ? g_hReplaceDlg : g_hFindDlg;
    BOOL matchCase = (fr->Flags & FR_MATCHCASE) != 0;
    BOOL wholeWord = (fr->Flags & FR_WHOLEWORD) != 0;
    if (fr->Flags & FR_FINDNEXT) {
        BOOL down = fromReplace || (fr->Flags & FR_DOWN) != 0;
        SearchEdit(edit, fr->lpstrFindWhat, down, matchCase, wholeWord, dlg);
    } else if (fr->Flags & FR_REPLACE) {
        ReplaceCurrent(edit, fr, dlg);
    } else if (fr->Flags & FR_REPLACEALL) {
        ReplaceAllInEdit(edit, fr->lpstrFindWhat, fr->lpstrReplaceWith, matchCase, wholeWord, dlg);
    }
}

static void InitFindReplace()
{
    ZeroMemory(&g_frFind, sizeof(g_frFind));
    g_frFind.lStructSize = sizeof(g_frFind);
    g_frFind.lpstrFindWhat = g_findWhat;
    g_frFind.wFindWhatLen = (WORD)sizeof(g_findWhat);
    ZeroMemory(&g_frReplace, sizeof(g_frReplace));
    g_frReplace.lStructSize = sizeof(g_frReplace);
    g_frReplace.lpstrFindWhat = g_findWhat;
    g_frReplace.wFindWhatLen = (WORD)sizeof(g_findWhat);
    g_frReplace.lpstrReplaceWith = g_replaceWith;
    g_frReplace.wReplaceWithLen = (WORD)sizeof(g_replaceWith);
}

static void PrefillFind(HWND edit)
{
    DWORD s = 0, e = 0;
    SendMessage(edit, EM_GETSEL, (WPARAM)&s, (LPARAM)&e);
    if (e > s && e - s < FIND_BUFFER_SIZE) {
        std::wstring text;
        GetEditString(edit, text);
        if (e <= text.size()) {
            std::wstring sub = text.substr(s, e - s);
            if (sub.find_first_of(L"\r\n") == NPOS) lstrcpyn(g_findWhat, sub.c_str(), FIND_BUFFER_SIZE);
        }
    }
}

static void ShowFindDialog(BOOL replace)
{
    HWND edit = ActiveEdit();
    if (edit) PrefillFind(edit);
    if (replace) {
        if (g_hFindDlg) {
            DestroyWindow(g_hFindDlg);
            g_hFindDlg = NULL;
        }
        if (g_hReplaceDlg) {
            SetFocus(g_hReplaceDlg);
            return;
        }
        g_frReplace.hwndOwner = g_hwndMain;
        g_frReplace.Flags = g_findFlags & (FR_MATCHCASE | FR_WHOLEWORD);
        g_hReplaceDlg = ReplaceText(&g_frReplace);
    } else {
        if (g_hReplaceDlg) {
            DestroyWindow(g_hReplaceDlg);
            g_hReplaceDlg = NULL;
        }
        if (g_hFindDlg) {
            SetFocus(g_hFindDlg);
            return;
        }
        g_frFind.hwndOwner = g_hwndMain;
        g_frFind.Flags = g_findFlags;
        g_hFindDlg = FindText(&g_frFind);
    }
}

static void DoFindNext(BOOL reverse)
{
    if (!g_findWhat[0]) {
        ShowFindDialog(FALSE);
        return;
    }
    BOOL down = (g_findFlags & FR_DOWN) != 0;
    if (reverse) down = !down;
    SearchEdit(ActiveEdit(), g_findWhat, down, (g_findFlags & FR_MATCHCASE) != 0,
               (g_findFlags & FR_WHOLEWORD) != 0, g_hwndMain);
}

static WORD* AlignDword(WORD* p)
{
    return (WORD*)(((ULONG_PTR)p + 3) & ~(ULONG_PTR)3);
}

/* Bounded template writers: g_dlgBuf is a fixed static buffer and the old
   helpers trusted their input, so a longer string would have walked off the
   end of it. They now return NULL when anything does not fit. */
static WORD* PutString(WORD* p, const wchar_t* s, WORD* end)
{
    for (;;) {
        if (p >= end) return NULL;
        WORD ch = (WORD)*s;
        *p++ = ch;
        if (ch == 0) return p;
        s++;
    }
}

static WORD* AddDlgItem(WORD* p, DWORD style, short x, short y, short cx, short cy,
                        WORD id, WORD cls, const wchar_t* text, WORD* end)
{
    if (!p) return NULL;
    p = AlignDword(p);
    if ((size_t)(end - p) < (sizeof(DLGITEMTEMPLATE) / sizeof(WORD) + 3)) return NULL;
    DLGITEMTEMPLATE* it = (DLGITEMTEMPLATE*)p;
    it->style = style | WS_CHILD | WS_VISIBLE;
    it->dwExtendedStyle = 0;
    it->x = x;
    it->y = y;
    it->cx = cx;
    it->cy = cy;
    it->id = id;
    p = (WORD*)(it + 1);
    *p++ = 0xFFFF;
    *p++ = cls;
    p = PutString(p, text, end);
    if (!p || p >= end) return NULL;
    *p++ = 0;
    return p;
}

static INT_PTR CALLBACK GotoDlgProc(HWND dlg, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG:
        SetDlgItemInt(dlg, IDC_GOTO_EDIT, (UINT)g_gotoValue, FALSE);
        SendDlgItemMessage(dlg, IDC_GOTO_EDIT, EM_SETSEL, 0, (LPARAM)-1);
        SetFocus(GetDlgItem(dlg, IDC_GOTO_EDIT));
        return FALSE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            BOOL ok = FALSE;
            UINT v = GetDlgItemInt(dlg, IDC_GOTO_EDIT, &ok, FALSE);
            if (!ok || v == 0) {
                MessageBeep(MB_ICONEXCLAMATION);
                return TRUE;
            }
            g_gotoValue = (int)v;
            EndDialog(dlg, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(dlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static void ShowGotoDialog()
{
    HWND edit = ActiveEdit();
    if (!edit) return;
    g_gotoValue = (int)SendMessage(edit, EM_LINEFROMCHAR, (WPARAM)-1, 0) + 1;
    ZeroMemory(g_dlgBuf, sizeof(g_dlgBuf));
    WORD* p = (WORD*)g_dlgBuf;
    WORD* end = (WORD*)((BYTE*)g_dlgBuf + sizeof(g_dlgBuf));
    DLGTEMPLATE* dt = (DLGTEMPLATE*)p;
    dt->style = DS_MODALFRAME | DS_CENTER | DS_SETFONT | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    dt->dwExtendedStyle = 0;
    dt->cdit = 4;
    dt->x = 0;
    dt->y = 0;
    dt->cx = 160;
    dt->cy = 64;
    p = (WORD*)(dt + 1);
    if (p + 2 >= end) return;
    *p++ = 0;
    *p++ = 0;
    p = PutString(p, L"Go To Line", end);
    if (!p || p >= end) return;
    *p++ = 8;
    p = PutString(p, L"MS Shell Dlg", end);
    if (!p) return;
    p = AddDlgItem(p, SS_LEFT, 8, 8, 144, 10, (WORD)0xFFFF, 0x0082, L"Line number:", end);
    p = AddDlgItem(p, WS_BORDER | WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL, 8, 20, 144, 14, IDC_GOTO_EDIT, 0x0081, L"", end);
    p = AddDlgItem(p, WS_TABSTOP | BS_DEFPUSHBUTTON, 40, 42, 50, 14, IDOK, 0x0080, L"Go To", end);
    p = AddDlgItem(p, WS_TABSTOP | BS_PUSHBUTTON, 100, 42, 50, 14, IDCANCEL, 0x0080, L"Cancel", end);
    if (!p) return;
    INT_PTR r = DialogBoxIndirectParam(g_hInst, (LPCDLGTEMPLATE)g_dlgBuf, g_hwndMain, GotoDlgProc, 0);
    if (r != IDOK) return;
    int total = (int)SendMessage(edit, EM_GETLINECOUNT, 0, 0);
    if (g_gotoValue > total) {
        MessageBox(g_hwndMain, L"The line number is beyond the total number of lines.", L"Go To Line", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int idx = (int)SendMessage(edit, EM_LINEINDEX, (WPARAM)(g_gotoValue - 1), 0);
    if (idx < 0) return;
    SendMessage(edit, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
    SendMessage(edit, EM_SCROLLCARET, 0, 0);
    SetFocus(edit);
}

static void InitPageSetup()
{
    ZeroMemory(&g_psd, sizeof(g_psd));
    g_psd.lStructSize = sizeof(g_psd);
    g_psd.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES;
    g_psd.rtMargin.left = 750;
    g_psd.rtMargin.top = 750;
    g_psd.rtMargin.right = 750;
    g_psd.rtMargin.bottom = 750;
}

static void DoPageSetup()
{
    g_psd.hwndOwner = g_hwndMain;
    if (!PageSetupDlg(&g_psd)) {
        if (CommDlgExtendedError()) {
            MessageBox(g_hwndMain, L"Unable to open Page Setup. Make sure a printer is installed.",
                       L"Notepad", MB_OK | MB_ICONERROR);
        }
    }
}

static int MarginToPixels(LONG v, int dpi)
{
    if (g_psd.Flags & PSD_INHUNDREDTHSOFMILLIMETERS) return MulDiv((int)v, dpi, 2540);
    return MulDiv((int)v, dpi, 1000);
}

static void PrintText(HDC hdc, const std::wstring& text, const wchar_t* docName)
{
    if (!hdc) return;
    DOCINFO di;
    ZeroMemory(&di, sizeof(di));
    di.cbSize = sizeof(di);
    di.lpszDocName = docName ? docName : L"Notepad";
    if (StartDoc(hdc, &di) <= 0) return;

    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    int offX = GetDeviceCaps(hdc, PHYSICALOFFSETX);
    int offY = GetDeviceCaps(hdc, PHYSICALOFFSETY);
    int pw = GetDeviceCaps(hdc, HORZRES);
    int ph = GetDeviceCaps(hdc, VERTRES);
    int physW = GetDeviceCaps(hdc, PHYSICALWIDTH);
    int physH = GetDeviceCaps(hdc, PHYSICALHEIGHT);
    int left = Max(0, MarginToPixels(g_psd.rtMargin.left, dpiX) - offX);
    int top = Max(0, MarginToPixels(g_psd.rtMargin.top, dpiY) - offY);
    int right = pw - Max(0, MarginToPixels(g_psd.rtMargin.right, dpiX) - (physW - pw - offX));
    int bottom = ph - Max(0, MarginToPixels(g_psd.rtMargin.bottom, dpiY) - (physH - ph - offY));
    if (right - left < 100 || bottom - top < 100) {
        left = 0;
        top = 0;
        right = pw;
        bottom = ph;
    }

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -MulDiv(10, dpiY, 72);
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    lstrcpyn(lf.lfFaceName, L"Courier New", LF_FACESIZE);
    HFONT font = CreateFontIndirect(&lf);

    /* StartPage resets the device context, so every DC attribute has to be
       applied after it and its result has to be checked. */
    if (StartPage(hdc) <= 0) {
        if (font) DeleteObject(font);
        EndDoc(hdc);
        return;
    }
    BOOL pageOpen = TRUE;
    HGDIOBJ oldFont = font ? SelectObject(hdc, font) : NULL;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    TEXTMETRIC tm;
    ZeroMemory(&tm, sizeof(tm));
    GetTextMetrics(hdc, &tm);
    int lineH = tm.tmHeight + tm.tmExternalLeading;
    if (lineH < 1) lineH = 1;

    int y = top;
    size_t pos = 0;
    for (;;) {
        size_t eol = text.find(L"\r\n", pos);
        size_t end = (eol == NPOS) ? text.size() : eol;
        size_t span = end - pos;
        /* DrawText takes an int for the length - never let a pathological line
           wrap the conversion. */
        int len = span > (size_t)0x7FFFFFFF ? 0x7FFFFFFF : (int)span;
        int h = lineH;
        if (len > 0) {
            RECT calc;
            calc.left = left;
            calc.top = 0;
            calc.right = right;
            calc.bottom = 0;
            DrawText(hdc, text.c_str() + pos, len, &calc, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS);
            h = Max(lineH, (int)(calc.bottom - calc.top));
        }
        if (y > top && y + h > bottom) {
            EndPage(hdc);
            pageOpen = FALSE;
            if (StartPage(hdc) <= 0) break;
            pageOpen = TRUE;
            if (font) SelectObject(hdc, font);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            y = top;
        }
        if (len > 0) {
            RECT dr;
            dr.left = left;
            dr.top = y;
            dr.right = right;
            dr.bottom = y + h;
            DrawText(hdc, text.c_str() + pos, len, &dr, DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS);
        }
        y += h;
        if (eol == NPOS) break;
        pos = eol + 2;
    }
    if (pageOpen) EndPage(hdc);
    EndDoc(hdc);
    if (oldFont) SelectObject(hdc, oldFont);
    if (font) DeleteObject(font);
}

static void DoPrint()
{
    HWND edit = ActiveEdit();
    if (!edit) return;
    PRINTDLG pd;
    ZeroMemory(&pd, sizeof(pd));
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = g_hwndMain;
    pd.hDevMode = g_psd.hDevMode;
    pd.hDevNames = g_psd.hDevNames;
    pd.Flags = PD_RETURNDC | PD_NOSELECTION | PD_NOPAGENUMS | PD_USEDEVMODECOPIESANDCOLLATE;
    pd.nCopies = 1;
    BOOL ok = PrintDlg(&pd);
    /* PrintDlg may free the handles we handed it even when it returns FALSE
       (user cancelled, or no printer is installed). Always copy back what the
       dialog leaves in pd: otherwise g_psd keeps a dangling HGLOBAL and the
       next Page Setup call works on freed memory. */
    g_psd.hDevMode = pd.hDevMode;
    g_psd.hDevNames = pd.hDevNames;
    if (!ok) {
        if (CommDlgExtendedError()) {
            MessageBox(g_hwndMain, L"Unable to print. Make sure a printer is installed.",
                       L"Notepad", MB_OK | MB_ICONERROR);
        }
        return;
    }
    if (!pd.hDC) return;
    std::wstring text;
    GetEditString(edit, text);
    wchar_t name[MAX_PATH_BUF + 4];
    BuildTabName(g_activeTab, name, MAX_PATH_BUF + 4, FALSE);
    PrintText(pd.hDC, text, name);
    DeleteDC(pd.hDC);
}

static RECT CloseRectFor(const RECT& r)
{
    RECT x;
    int sz = Scale(16);
    x.right = r.right - Scale(6);
    x.left = x.right - sz;
    x.top = (r.top + r.bottom) / 2 - sz / 2;
    x.bottom = x.top + sz;
    return x;
}

static void DrawTabItem(const DRAWITEMSTRUCT* dis)
{
    int idx = (int)dis->itemID;
    if (idx < 0 || idx >= (int)g_tabs.size()) return;
    HDC dc = dis->hDC;
    RECT r = dis->rcItem;
    BOOL sel = (idx == g_activeTab);
    FillRect(dc, &r, GetSysColorBrush(sel ? COLOR_WINDOW : COLOR_BTNFACE));
    if (sel) {
        RECT ar = r;
        ar.bottom = ar.top + 2;
        FillRect(dc, &ar, GetSysColorBrush(COLOR_HIGHLIGHT));
    }

    HPEN sepPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
    HGDIOBJ oldPen = SelectObject(dc, sepPen);
    MoveToEx(dc, r.right - 1, r.top + Scale(5), NULL);
    LineTo(dc, r.right - 1, r.bottom - Scale(5));
    SelectObject(dc, oldPen);
    DeleteObject(sepPen);

    RECT xr = CloseRectFor(r);
    HPEN xPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
    oldPen = SelectObject(dc, xPen);
    int pad = Scale(4);
    MoveToEx(dc, xr.left + pad, xr.top + pad, NULL);
    LineTo(dc, xr.right - pad, xr.bottom - pad);
    MoveToEx(dc, xr.right - pad - 1, xr.top + pad, NULL);
    LineTo(dc, xr.left + pad - 1, xr.bottom - pad);
    SelectObject(dc, oldPen);
    DeleteObject(xPen);

    wchar_t name[MAX_PATH_BUF + 4];
    BuildTabName(idx, name, MAX_PATH_BUF + 4, TRUE);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
    HGDIOBJ oldFont = SelectObject(dc, g_hUIFont);
    RECT tr = r;
    tr.left += Scale(10);
    tr.right = xr.left - Scale(4);
    DrawText(dc, name, -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
    SelectObject(dc, oldFont);
}

static int TabHitIndex(POINT pt, BOOL* onClose)
{
    int n = (int)g_tabs.size();
    if (onClose) *onClose = FALSE;
    for (int i = 0; i < n; i++) {
        RECT r;
        if (!SendMessage(g_hwndTab, TCM_GETITEMRECT, (WPARAM)i, (LPARAM)&r)) continue;
        if (PtInRect(&r, pt)) {
            if (onClose) {
                RECT xr = CloseRectFor(r);
                InflateRect(&xr, Scale(3), Scale(3));
                *onClose = PtInRect(&xr, pt) ? TRUE : FALSE;
            }
            return i;
        }
    }
    return -1;
}

static HMENU BuildMenu()
{
    HMENU bar = CreateMenu();
    HMENU file = CreatePopupMenu();
    HMENU edit = CreatePopupMenu();
    HMENU enc = CreatePopupMenu();
    HMENU view = CreatePopupMenu();
    HMENU help = CreatePopupMenu();

    AppendMenu(file, MF_STRING, IDM_FILE_NEW, L"&New Tab\tCtrl+N");
    AppendMenu(file, MF_STRING, IDM_FILE_OPEN, L"&Open...\tCtrl+O");
    AppendMenu(file, MF_STRING, IDM_FILE_SAVE, L"&Save\tCtrl+S");
    AppendMenu(file, MF_STRING, IDM_FILE_SAVEAS, L"Save &As...\tCtrl+Shift+S");
    AppendMenu(file, MF_STRING, IDM_FILE_CLOSETAB, L"&Close Tab\tCtrl+W");
    AppendMenu(file, MF_SEPARATOR, 0, NULL);
    AppendMenu(file, MF_STRING, IDM_FILE_PAGESETUP, L"Page Set&up...");
    AppendMenu(file, MF_STRING, IDM_FILE_PRINT, L"&Print...\tCtrl+P");
    AppendMenu(file, MF_SEPARATOR, 0, NULL);
    AppendMenu(file, MF_STRING, IDM_FILE_EXIT, L"E&xit");

    AppendMenu(edit, MF_STRING, IDM_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenu(edit, MF_SEPARATOR, 0, NULL);
    AppendMenu(edit, MF_STRING, IDM_EDIT_CUT, L"Cu&t\tCtrl+X");
    AppendMenu(edit, MF_STRING, IDM_EDIT_COPY, L"&Copy\tCtrl+C");
    AppendMenu(edit, MF_STRING, IDM_EDIT_PASTE, L"&Paste\tCtrl+V");
    AppendMenu(edit, MF_STRING, IDM_EDIT_DELETE, L"De&lete\tDel");
    AppendMenu(edit, MF_SEPARATOR, 0, NULL);
    AppendMenu(edit, MF_STRING, IDM_EDIT_FIND, L"&Find...\tCtrl+F");
    AppendMenu(edit, MF_STRING, IDM_EDIT_FINDNEXT, L"Find &Next\tF3");
    AppendMenu(edit, MF_STRING, IDM_EDIT_FINDPREV, L"Find Pre&vious\tShift+F3");
    AppendMenu(edit, MF_STRING, IDM_EDIT_REPLACE, L"&Replace...\tCtrl+H");
    AppendMenu(edit, MF_STRING, IDM_EDIT_GOTO, L"&Go To...\tCtrl+G");
    AppendMenu(edit, MF_SEPARATOR, 0, NULL);
    AppendMenu(edit, MF_STRING, IDM_EDIT_SELECTALL, L"Select &All\tCtrl+A");
    AppendMenu(edit, MF_STRING, IDM_EDIT_TIME, L"Time/&Date\tF5");

    AppendMenu(enc, MF_STRING, IDM_ENC_ANSI, L"&ANSI");
    AppendMenu(enc, MF_STRING, IDM_ENC_UTF8, L"UTF-&8");
    AppendMenu(enc, MF_STRING, IDM_ENC_UTF8BOM, L"UTF-8 with &BOM");
    AppendMenu(enc, MF_STRING, IDM_ENC_UTF16LE, L"UTF-16 &LE");
    AppendMenu(enc, MF_STRING, IDM_ENC_UTF16BE, L"UTF-16 B&E");

    AppendMenu(view, MF_STRING, IDM_VIEW_WRAP, L"&Word Wrap");
    AppendMenu(view, MF_STRING, IDM_VIEW_STATUSBAR, L"&Status Bar");
    AppendMenu(view, MF_SEPARATOR, 0, NULL);
    AppendMenu(view, MF_STRING, IDM_TAB_NEXT, L"&Next Tab\tCtrl+Tab");
    AppendMenu(view, MF_STRING, IDM_TAB_PREV, L"&Previous Tab\tCtrl+Shift+Tab");

    AppendMenu(help, MF_STRING, IDM_HELP_ABOUT, L"&About Notepad");

    AppendMenu(bar, MF_POPUP, (UINT_PTR)file, L"&File");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)edit, L"&Edit");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)enc, L"En&coding");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)view, L"&View");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)help, L"&Help");

    g_hEditMenu = edit;
    g_hEncMenu = enc;
    g_hViewMenu = view;
    return bar;
}

static HACCEL CreateAccelerators()
{
    ACCEL a[] = {
        { FVIRTKEY | FCONTROL, 'N', IDM_FILE_NEW },
        { FVIRTKEY | FCONTROL, 'O', IDM_FILE_OPEN },
        { FVIRTKEY | FCONTROL, 'S', IDM_FILE_SAVE },
        { FVIRTKEY | FCONTROL | FSHIFT, 'S', IDM_FILE_SAVEAS },
        { FVIRTKEY | FCONTROL, 'W', IDM_FILE_CLOSETAB },
        { FVIRTKEY | FCONTROL, 'P', IDM_FILE_PRINT },
        { FVIRTKEY | FCONTROL, 'F', IDM_EDIT_FIND },
        { FVIRTKEY, VK_F3, IDM_EDIT_FINDNEXT },
        { FVIRTKEY | FSHIFT, VK_F3, IDM_EDIT_FINDPREV },
        { FVIRTKEY | FCONTROL, 'H', IDM_EDIT_REPLACE },
        { FVIRTKEY | FCONTROL, 'G', IDM_EDIT_GOTO },
        { FVIRTKEY | FCONTROL, 'Z', IDM_EDIT_UNDO },
        { FVIRTKEY | FCONTROL, 'X', IDM_EDIT_CUT },
        { FVIRTKEY | FCONTROL, 'C', IDM_EDIT_COPY },
        { FVIRTKEY | FCONTROL, 'V', IDM_EDIT_PASTE },
        { FVIRTKEY | FCONTROL, 'A', IDM_EDIT_SELECTALL },
        { FVIRTKEY, VK_F5, IDM_EDIT_TIME },
        { FVIRTKEY | FCONTROL, VK_TAB, IDM_TAB_NEXT },
        { FVIRTKEY | FCONTROL | FSHIFT, VK_TAB, IDM_TAB_PREV }
    };
    return CreateAcceleratorTable(a, (int)(sizeof(a) / sizeof(a[0])));
}

static void InsertTimeDate(HWND edit)
{
    wchar_t t[64];
    wchar_t d[64];
    t[0] = L'\0';
    d[0] = L'\0';
    GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, t, 64);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, d, 64);
    std::wstring s = t;
    s += L" ";
    s += d;
    SendMessage(edit, EM_REPLACESEL, TRUE, (LPARAM)s.c_str());
}

static void SetTabEncoding(int enc)
{
    if (g_activeTab < 0) return;
    if (g_tabs[(size_t)g_activeTab].encoding == enc) return;
    g_tabs[(size_t)g_activeTab].encoding = enc;
    SetTabModified(g_activeTab, TRUE);
    UpdateMenuChecks();
    UpdateStatusBar();
}

/* True while the caret sits inside the modeless Find/Replace box. Used to
   keep menu accelerators from stealing keystrokes the user is typing there
   (Ctrl+A used to select all in the *editor* instead of the search field). */
static BOOL FocusInChildDialog()
{
    HWND f = GetFocus();
    if (!f) return FALSE;
    if (g_hFindDlg && (f == g_hFindDlg || IsChild(g_hFindDlg, f))) return TRUE;
    if (g_hReplaceDlg && (f == g_hReplaceDlg || IsChild(g_hReplaceDlg, f))) return TRUE;
    return FALSE;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (g_findMsg && msg == g_findMsg) {
        HandleFindMessage((LPFINDREPLACE)lp);
        return 0;
    }

    switch (msg) {
    case WM_CREATE: {
        g_hwndMain = hwnd;
        g_hUIFont = CreateUIFont();
        g_hEditFont = CreateEditFont();
        g_hwndTab = CreateWindowEx(0, WC_TABCONTROL, L"",
                                   WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_OWNERDRAWFIXED | TCS_FIXEDWIDTH | TCS_FOCUSNEVER,
                                   0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)IDC_TAB_CTRL, g_hInst, NULL);
        if (!g_hwndTab) return -1;
        if (g_hUIFont) SendMessage(g_hwndTab, WM_SETFONT, (WPARAM)g_hUIFont, FALSE);
        SendMessage(g_hwndTab, TCM_SETITEMSIZE, 0, MAKELPARAM(Scale(TAB_WIDTH), Scale(TAB_HEIGHT)));
        g_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                      0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)IDC_STATUS, g_hInst, NULL);
        if (!g_hwndStatus) return -1;
        if (g_hUIFont) SendMessage(g_hwndStatus, WM_SETFONT, (WPARAM)g_hUIFont, FALSE);
        if (!SetMenu(hwnd, BuildMenu())) return -1;
        g_hAccel = CreateAccelerators();
        UpdateMenuChecks();
        DragAcceptFiles(hwnd, TRUE);
        SetTimer(hwnd, 1, 200, NULL);
        return 0;
    }

    case WM_SIZE: {
        if (wp != SIZE_MINIMIZED) LayoutChildren();
        return 0;
    }

    case WM_APP_REDRAW_STATUS: {
        if (g_hwndStatus) {
            RedrawWindow(g_hwndStatus, NULL, NULL,
                         RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
        }
        return 0;
    }

    case WM_SETFOCUS: {
        HWND e = ActiveEdit();
        if (e) SetFocus(e);
        return 0;
    }

    case WM_TIMER: {
        /* Only refresh the caret readout when it can actually be seen;
           polling a hidden status bar just burns CPU. */
        if (g_showStatus && IsWindowVisible(hwnd)) UpdateStatusBar();
        return 0;
    }

    case WM_DRAWITEM: {
        if (wp == IDC_TAB_CTRL) {
            DrawTabItem((const DRAWITEMSTRUCT*)lp);
            return TRUE;
        }
        break;
    }

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lp;
        if (nm->hwndFrom == g_hwndTab) {
            if (nm->code == TCN_SELCHANGE) {
                SelectTab((int)SendMessage(g_hwndTab, TCM_GETCURSEL, 0, 0));
            } else if (nm->code == NM_CLICK || nm->code == NM_DBLCLK) {
                DWORD mp = GetMessagePos();
                POINT pt;
                pt.x = (short)LOWORD(mp);
                pt.y = (short)HIWORD(mp);
                ScreenToClient(g_hwndTab, &pt);
                BOOL onClose = FALSE;
                int hit = TabHitIndex(pt, &onClose);
                if (nm->code == NM_CLICK) {
                    if (hit >= 0 && onClose) {
                        /* A double click delivers NM_CLICK twice at (almost)
                           the same point. Without this guard the second click
                           hit the tab that had shifted into the close box and
                           silently destroyed it as well. */
                        DWORD now = GetTickCount();
                        int dx = pt.x - g_lastClosePtX;
                        int dy = pt.y - g_lastClosePtY;
                        if (dx < 0) dx = -dx;
                        if (dy < 0) dy = -dy;
                        BOOL repeat = (g_lastCloseTick != 0) &&
                                      (now - g_lastCloseTick <= GetDoubleClickTime()) &&
                                      dx <= 4 && dy <= 4;
                        if (!repeat) {
                            g_lastCloseTick = now;
                            g_lastClosePtX = pt.x;
                            g_lastClosePtY = pt.y;
                            CloseTab(hit);
                        }
                    }
                } else if (hit < 0) {
                    AddNewTab();
                }
            }
        }
        break;
    }

    case WM_INITMENUPOPUP: {
        if ((HMENU)wp == g_hEditMenu) {
            HWND e = ActiveEdit();
            BOOL hasSel = FALSE;
            BOOL canUndo = FALSE;
            if (e) {
                DWORD s = 0, en = 0;
                SendMessage(e, EM_GETSEL, (WPARAM)&s, (LPARAM)&en);
                hasSel = (s != en);
                canUndo = SendMessage(e, EM_CANUNDO, 0, 0) ? TRUE : FALSE;
            }
            HMENU m = g_hEditMenu;
            EnableMenuItem(m, IDM_EDIT_UNDO, MF_BYCOMMAND | (canUndo ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(m, IDM_EDIT_CUT, MF_BYCOMMAND | (hasSel ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(m, IDM_EDIT_COPY, MF_BYCOMMAND | (hasSel ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(m, IDM_EDIT_DELETE, MF_BYCOMMAND | (hasSel ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(m, IDM_EDIT_PASTE, MF_BYCOMMAND | (IsClipboardFormatAvailable(CF_UNICODETEXT) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(m, IDM_EDIT_FINDNEXT, MF_BYCOMMAND | (g_findWhat[0] ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(m, IDM_EDIT_FINDPREV, MF_BYCOMMAND | (g_findWhat[0] ? MF_ENABLED : MF_GRAYED));
            return 0;
        }
        break;
    }

    case WM_DROPFILES: {
        HDROP hd = (HDROP)wp;
        UINT n = DragQueryFile(hd, 0xFFFFFFFF, NULL, 0);
        for (UINT i = 0; i < n; i++) {
            wchar_t p[MAX_PATH_BUF];
            if (DragQueryFile(hd, i, p, MAX_PATH_BUF)) {
                DWORD a = GetFileAttributes(p);
                if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) OpenPath(p);
            }
        }
        DragFinish(hd);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp);
        int code = HIWORD(wp);
        if (id == IDC_EDIT && lp) {
            if (code == EN_CHANGE && g_suppressChange == 0) {
                for (size_t i = 0; i < g_tabs.size(); i++) {
                    if (g_tabs[i].hwndEdit == (HWND)lp) {
                        SetTabModified((int)i, TRUE);
                        break;
                    }
                }
                UpdateStatusBar();
            }
            return 0;
        }
        HWND e = ActiveEdit();
        switch (id) {
        case IDM_FILE_NEW:
            AddNewTab();
            break;
        case IDM_FILE_OPEN: {
            wchar_t p[MAX_PATH_BUF];
            if (ShowOpenDialog(hwnd, p, MAX_PATH_BUF)) OpenPath(p);
            break;
        }
        case IDM_FILE_SAVE:
            SaveTab(g_activeTab, FALSE);
            break;
        case IDM_FILE_SAVEAS:
            SaveTab(g_activeTab, TRUE);
            break;
        case IDM_FILE_CLOSETAB:
            CloseTab(g_activeTab);
            break;
        case IDM_FILE_PAGESETUP:
            DoPageSetup();
            break;
        case IDM_FILE_PRINT:
            DoPrint();
            break;
        case IDM_FILE_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case IDM_EDIT_UNDO:
            if (e) SendMessage(e, EM_UNDO, 0, 0);
            break;
        case IDM_EDIT_CUT:
            if (e) SendMessage(e, WM_CUT, 0, 0);
            break;
        case IDM_EDIT_COPY:
            if (e) SendMessage(e, WM_COPY, 0, 0);
            break;
        case IDM_EDIT_PASTE:
            if (e) SendMessage(e, WM_PASTE, 0, 0);
            break;
        case IDM_EDIT_DELETE:
            if (e) SendMessage(e, WM_CLEAR, 0, 0);
            break;
        case IDM_EDIT_SELECTALL:
            if (e) SendMessage(e, EM_SETSEL, 0, (LPARAM)-1);
            break;
        case IDM_EDIT_TIME:
            if (e) InsertTimeDate(e);
            break;
        case IDM_EDIT_FIND:
            ShowFindDialog(FALSE);
            break;
        case IDM_EDIT_REPLACE:
            ShowFindDialog(TRUE);
            break;
        case IDM_EDIT_FINDNEXT:
            DoFindNext(FALSE);
            break;
        case IDM_EDIT_FINDPREV:
            DoFindNext(TRUE);
            break;
        case IDM_EDIT_GOTO:
            ShowGotoDialog();
            break;
        case IDM_ENC_ANSI:
            SetTabEncoding(ENC_ANSI);
            break;
        case IDM_ENC_UTF8:
            SetTabEncoding(ENC_UTF8);
            break;
        case IDM_ENC_UTF8BOM:
            SetTabEncoding(ENC_UTF8_BOM);
            break;
        case IDM_ENC_UTF16LE:
            SetTabEncoding(ENC_UTF16LE);
            break;
        case IDM_ENC_UTF16BE:
            SetTabEncoding(ENC_UTF16BE);
            break;
        case IDM_VIEW_WRAP:
            g_wrap = !g_wrap;
            ApplyWrapToAllTabs();
            UpdateMenuChecks();
            break;
        case IDM_VIEW_STATUSBAR:
            g_showStatus = !g_showStatus;
            ShowWindow(g_hwndStatus, g_showStatus ? SW_SHOW : SW_HIDE);
            LayoutChildren();
            UpdateMenuChecks();
            break;
        case IDM_TAB_NEXT:
            if (g_tabs.size() > 1) SelectTab((g_activeTab + 1) % (int)g_tabs.size());
            break;
        case IDM_TAB_PREV:
            if (g_tabs.size() > 1) SelectTab((g_activeTab + (int)g_tabs.size() - 1) % (int)g_tabs.size());
            break;
        case IDM_HELP_ABOUT:
            MessageBox(hwnd, L"Notepad\nTabbed text editor for Windows 7 and later", L"About Notepad", MB_OK | MB_ICONINFORMATION);
            break;
        }
        return 0;
    }

    case WM_CLOSE: {
        for (int i = 0; i < (int)g_tabs.size(); i++) {
            if (g_tabs[(size_t)i].modified) {
                SelectTab(i);
                if (!ConfirmSave(i)) return 0;
            }
        }
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        /* The Find/Replace windows are owned by this window, so Windows
           destroys them right after this message. Drop the handles so that
           nothing touches them again. */
        g_hFindDlg = NULL;
        g_hReplaceDlg = NULL;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

static int RunNotepad(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    (void)hPrev;
    (void)lpCmd;
    g_hInst = hInstance;
    if (!LoadDynamicApis()) {
        MessageBoxW(NULL, L"Required system libraries could not be loaded.", L"Notepad", MB_OK | MB_ICONERROR);
        return 1;
    }

    /* Become system-DPI aware before any window or font is created, so every
       Scale()/font size below works in real pixels. The manifest declares the
       same thing; this call is the fallback for builds without a resource
       section and is a harmless no-op when the manifest already applied it. */
    SetProcessDPIAware();
    {
        HDC dc = GetDC(NULL);
        int dpi = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(NULL, dc);
        if (dpi > 0) g_dpi = dpi;
    }

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    g_findMsg = RegisterWindowMessage(FINDMSGSTRING);
    InitFindReplace();
    InitPageSetup();

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"NotepadClass";
    if (!RegisterClassEx(&wc)) {
        MessageBoxW(NULL, L"Could not register the window class.", L"Notepad", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(0, L"NotepadClass", L"Notepad",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT, Scale(960), Scale(640),
                               NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBoxW(NULL, L"Could not create the main window.", L"Notepad", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    /* Open the command-line files only after the window exists, so any error
       box has a valid owner and the tab layout runs against a real client
       rectangle instead of a 0x0 one. */
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        for (int i = 1; i < argc; i++) OpenPath(argv[i]);
        LocalFree(argv);
    }
    if (g_tabs.empty()) AddNewTab();

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    BOOL gm;
    /* GetMessage returns 0 for WM_QUIT and -1 on error; the old "> 0" test
       treated an error as a normal quit and then read an indeterminate
       msg.wParam as the exit code. */
    while ((gm = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (gm == -1) break;
        if (g_hFindDlg && IsDialogMessage(g_hFindDlg, &msg)) continue;
        if (g_hReplaceDlg && IsDialogMessage(g_hReplaceDlg, &msg)) continue;
        if (!FocusInChildDialog() && g_hAccel && TranslateAccelerator(hwnd, g_hAccel, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_hAccel) DestroyAcceleratorTable(g_hAccel);
    if (g_hEditFont) DeleteObject(g_hEditFont);
    if (g_hUIFont) DeleteObject(g_hUIFont);
    if (g_psd.hDevMode) GlobalFree(g_psd.hDevMode);
    if (g_psd.hDevNames) GlobalFree(g_psd.hDevNames);
    return (int)msg.wParam;
}

/* Nothing in this program should ever be able to take the process down with an
   unhandled C++ exception: DispatchMessage() runs our window procedures, so an
   allocation failure anywhere in the UI would otherwise reach
   std::terminate() and silently lose every unsaved tab. */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    try {
        return RunNotepad(hInstance, hPrev, lpCmd, nShow);
    } catch (...) {
        MessageBoxW(NULL,
                    L"An unexpected error occurred and the operation was aborted.\n"
                    L"Any documents you had open were not modified.",
                    L"Notepad", MB_OK | MB_ICONERROR);
        return 1;
    }
}