$src = @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class S {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr Send(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr SendStr(IntPtr h, uint m, IntPtr w, [MarshalAs(UnmanagedType.LPWStr)] string l);
}
'@
Add-Type -TypeDefinition $src -ErrorAction Stop

$script:mainHwnd = [IntPtr]::Zero
function Find-MainWindow([int]$procId) {
  $script:mainHwnd = [IntPtr]::Zero
  $want = [uint32]$procId
  $cb = [S+EnumProc]{ param($h, $l)
    $owning = [uint32]0
    [void][S]::GetWindowThreadProcessId($h, [ref]$owning)
    if ($owning -eq $want) {
      $sb = New-Object Text.StringBuilder 128
      [void][S]::GetClassName($h, $sb, 128)
      if ($sb.ToString() -eq 'NotepadClass') { $script:mainHwnd = $h; return $false }
    }
    return $true
  }
  [void][S]::EnumWindows($cb, [IntPtr]::Zero)
  return $script:mainHwnd
}

$script:editHwnd = [IntPtr]::Zero
function Find-Edit([IntPtr]$main) {
  $script:editHwnd = [IntPtr]::Zero
  $cb = [S+EnumProc]{ param($h, $l)
    $sb = New-Object Text.StringBuilder 64
    [void][S]::GetClassName($h, $sb, 64)
    if ($sb.ToString() -eq 'EDIT' -and [S]::IsWindowVisible($h)) { $script:editHwnd = $h; return $false }
    return $true
  }
  [void][S]::EnumChildWindows($main, $cb, [IntPtr]::Zero)
  return $script:editHwnd
}

# menu command ids used by main.cpp
$IDM_FILE_SAVE = 3103
$newText = 'Hello' + "`r`n" + 'World'

# Each entry yields an encoding factory; WriteAllText emits the preamble, so
# the expected bytes are preamble + GetBytes() in every case.
$cases = @(
  @{ n = 'ansi';    mk = { New-Object Text.ASCIIEncoding } },
  @{ n = 'utf8';    mk = { New-Object Text.UTF8Encoding($false) } },
  @{ n = 'utf8bom'; mk = { New-Object Text.UTF8Encoding($true) } },
  @{ n = 'utf16le'; mk = { New-Object Text.UnicodeEncoding($false, $true) } },
  @{ n = 'utf16be'; mk = { New-Object Text.UnicodeEncoding($true, $true) } }
)

$fail = 0
foreach ($c in $cases) {
  $path = Join-Path (Get-Location) ("roundtrip_" + $c.n + ".txt")
  $initial = 'original ' + $c.n + ' content'
  [IO.File]::WriteAllText($path, $initial, (& $c.mk))

  $p = Start-Process -FilePath (Join-Path (Get-Location) 'Notepad.exe') -ArgumentList ('"' + $path + '"') -PassThru
  Start-Sleep -Milliseconds 1600
  $main = Find-MainWindow $p.Id
  $edit = Find-Edit $main
  if ($edit -eq [IntPtr]::Zero) {
    Write-Output ("{0,-10} FAIL no edit control" -f $c.n); $fail++
    if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }
    continue
  }
  # replace the document, then issue File->Save
  [void][S]::SendStr($edit, 0x000C, [IntPtr]::Zero, $newText)   # WM_SETTEXT
  Start-Sleep -Milliseconds 300
  [void][S]::Send($main, 0x0111, [IntPtr]$IDM_FILE_SAVE, [IntPtr]::Zero)  # WM_COMMAND
  Start-Sleep -Milliseconds 700
  if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }

  $got = [IO.File]::ReadAllBytes($path)
  $e2 = & $c.mk
  $exp = $e2.GetPreamble() + $e2.GetBytes($newText)
  $ok = ($got.Length -eq $exp.Length)
  if ($ok) { for ($i = 0; $i -lt $got.Length; $i++) { if ($got[$i] -ne $exp[$i]) { $ok = $false; break } } }
  if (-not $ok) { $fail++ }
  $hex = ($got | Select-Object -First 12 | ForEach-Object { $_.ToString('X2') }) -join ' '
  Write-Output ("{0,-10} match={1,-5} len={2} firstBytes=[{3}]" -f $c.n, $ok, $got.Length, $hex)
}
Write-Output "FAILURES=$fail"
if ($fail -gt 0) { exit 1 }
exit 0
