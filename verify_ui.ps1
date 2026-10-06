$src = @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class U {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll")] public static extern IntPtr FindWindowEx(IntPtr parent, IntPtr after, string cls, string title);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr Send(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr SendStr(IntPtr h, uint m, IntPtr w, [MarshalAs(UnmanagedType.LPWStr)] string l);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr SendBuf(IntPtr h, uint m, IntPtr w, [Out] StringBuilder l);
}
'@
Add-Type -TypeDefinition $src -ErrorAction Stop

$script:mainHwnd = [IntPtr]::Zero
function Find-MainWindow([int]$procId) {
  $script:mainHwnd = [IntPtr]::Zero
  $want = [uint32]$procId
  $cb = [U+EnumProc]{ param($h, $l)
    $owning = [uint32]0
    [void][U]::GetWindowThreadProcessId($h, [ref]$owning)
    if ($owning -eq $want) {
      $sb = New-Object Text.StringBuilder 128
      [void][U]::GetClassName($h, $sb, 128)
      if ($sb.ToString() -eq 'NotepadClass') { $script:mainHwnd = $h; return $false }
    }
    return $true
  }
  [void][U]::EnumWindows($cb, [IntPtr]::Zero)
  return $script:mainHwnd
}
function Find-Edit([IntPtr]$main) {
  $script:editHwnd = [IntPtr]::Zero
  $cb = [U+EnumProc]{ param($h, $l)
    $sb = New-Object Text.StringBuilder 64
    [void][U]::GetClassName($h, $sb, 64)
    if ($sb.ToString() -eq 'EDIT' -and [U]::IsWindowVisible($h)) { $script:editHwnd = $h; return $false }
    return $true
  }
  [void][U]::EnumChildWindows($main, $cb, [IntPtr]::Zero)
  return $script:editHwnd
}
function Get-EditText([IntPtr]$edit) {
  if ($edit -eq [IntPtr]::Zero) { return $null }
  $len = [int][U]::Send($edit, 0x000E, [IntPtr]::Zero, [IntPtr]::Zero)
  if ($len -le 0) { return '' }
  $s = New-Object Text.StringBuilder ($len + 1)
  [void][U]::SendBuf($edit, 0x000D, [IntPtr]($len + 1), $s)
  return $s.ToString()
}
function Get-Title([IntPtr]$main) {
  $s = New-Object Text.StringBuilder 512
  [void][U]::GetWindowText($main, $s, 512)
  return $s.ToString()
}
function Cmd([IntPtr]$main, [int]$id) {
  [void][U]::Send($main, 0x0111, [IntPtr]$id, [IntPtr]::Zero)
}

$IDM_FILE_NEW      = 3101
$IDM_FILE_CLOSETAB = 3105
$IDM_EDIT_TIME     = 3212
$IDM_EDIT_SELALL   = 3211
$IDM_VIEW_WRAP     = 3401
$IDM_VIEW_STATUS   = 3402
$IDM_TAB_NEXT      = 3403

$path = Join-Path (Get-Location) 'ui_test.txt'
$payload = 'alpha' + "`r`n" + 'beta' + "`r`n" + 'gamma'
[IO.File]::WriteAllText($path, $payload, (New-Object Text.UTF8Encoding($false)))

$p = Start-Process -FilePath (Join-Path (Get-Location) 'Notepad.exe') -ArgumentList ('"' + $path + '"') -PassThru
Start-Sleep -Milliseconds 1600
$main = Find-MainWindow $p.Id
$fail = 0
function Check($name, $cond, $detail) {
  if ($cond) { Write-Output ("{0,-34} PASS  {1}" -f $name, $detail) }
  else       { Write-Output ("{0,-34} FAIL  {1}" -f $name, $detail); $script:fail++ }
}

$edit = Find-Edit $main
Check 'file loaded' ((Get-EditText $edit) -ceq $payload) ((Get-Title $main))

# --- word wrap round trip must preserve the document exactly
Cmd $main $IDM_VIEW_WRAP
Start-Sleep -Milliseconds 400
$edit = Find-Edit $main
$t1 = Get-EditText $edit
Cmd $main $IDM_VIEW_WRAP
Start-Sleep -Milliseconds 400
$edit = Find-Edit $main
$t2 = Get-EditText $edit
Check 'word wrap on/off preserves text' ($t1 -ceq $payload -and $t2 -ceq $payload) ("len1=" + $t1.Length + " len2=" + $t2.Length)

# --- status bar round trip leaves the app alive and text intact
Cmd $main $IDM_VIEW_STATUS
Start-Sleep -Milliseconds 250
Cmd $main $IDM_VIEW_STATUS
Start-Sleep -Milliseconds 250
$edit = Find-Edit $main
Check 'status bar toggle' ($p.HasExited -eq $false -and (Get-EditText $edit) -ceq $payload) ('alive=' + (-not $p.HasExited))

# --- editing marks the tab dirty (title gets a leading asterisk)
$before = Get-Title $main
Cmd $main $IDM_EDIT_TIME
Start-Sleep -Milliseconds 400
$after = Get-Title $main
Check 'edit marks tab modified' ($before -ne $after -and $after.StartsWith('*')) ("before=[$before] after=[$after]")

# --- new tab / close the new tab (it is Untitled and unmodified, so closing
#     it must not raise the "save changes?" prompt)
$edit = Find-Edit $main
$textBefore = Get-EditText $edit
Cmd $main $IDM_FILE_NEW
Start-Sleep -Milliseconds 500
Check 'new tab added' ((Get-Title $main) -match 'Untitled') ((Get-Title $main))
Cmd $main $IDM_FILE_CLOSETAB
Start-Sleep -Milliseconds 700
$edit = Find-Edit $main
$afterClose = Get-EditText $edit
Check 'closing untitled tab returns to document' ($p.HasExited -eq $false -and $afterClose -ceq $textBefore) ("alive=" + (-not $p.HasExited) + " title=" + (Get-Title $main))

if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }
Write-Output "FAILURES=$fail"
if ($fail -gt 0) { exit 1 }
exit 0
