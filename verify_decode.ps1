$src = @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class Win {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, EnumProc cb, IntPtr l);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr SendLen(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
    public static extern IntPtr SendTxt(IntPtr h, uint m, IntPtr w, [Out] StringBuilder l);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string c, string t);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
}
'@
Add-Type -TypeDefinition $src -ErrorAction Stop

$script:dfound = [IntPtr]::Zero

# Find the app's top-level window for a given pid: FindWindow with a null
# title does not match reliably across processes here, so enumerate instead.
function Find-MainWindow([int]$procId) {
  $script:mainHwnd = [IntPtr]::Zero
  $want = [uint32]$procId
  $cb = [Win+EnumProc]{ param($h, $l)
    $owning = [uint32]0
    [void][Win]::GetWindowThreadProcessId($h, [ref]$owning)
    if ($owning -eq $want) {
      $sb = New-Object System.Text.StringBuilder 128
      [void][Win]::GetClassName($h, $sb, 128)
      if ($sb.ToString() -eq 'NotepadClass') { $script:mainHwnd = $h; return $false }
    }
    return $true
  }
  [void][Win]::EnumWindows($cb, [IntPtr]::Zero)
  return $script:mainHwnd
}

function Get-EditText([IntPtr]$main) {
  $script:found = [IntPtr]::Zero
  $cb = [Win+EnumProc]{ param($h, $l)
    $sb = New-Object System.Text.StringBuilder 64
    [void][Win]::GetClassName($h, $sb, 64)
    if ($sb.ToString() -eq 'EDIT' -and [Win]::IsWindowVisible($h)) { $script:found = $h; return $false }
    return $true
  }
  [void][Win]::EnumChildWindows($main, $cb, [IntPtr]::Zero)
  $h = $script:found
  if ($h -eq [IntPtr]::Zero) { return $null }
  $len = [int][Win]::SendLen($h, 0x000E, [IntPtr]::Zero, [IntPtr]::Zero)
  if ($len -le 0) { return '' }
  $s = New-Object System.Text.StringBuilder ($len + 1)
  [void][Win]::SendTxt($h, 0x000D, [IntPtr]($len + 1), $s)
  return $s.ToString()
}

$utf8Txt  = 'caf' + [char]0x00E9 + ' ' + [char]0x4E2D + [char]0x6587
$asciiTxt = 'hello world'
$multi    = 'line one' + "`r`n" + 'line two'

# --- build the sample files (PowerShell 5.1 compatible, no `u{...}` escapes)
[IO.File]::WriteAllText((Join-Path (Get-Location) 't_ascii.txt'), $asciiTxt, (New-Object Text.ASCIIEncoding))
[IO.File]::WriteAllText((Join-Path (Get-Location) 't_utf8.txt'), $utf8Txt, (New-Object Text.UTF8Encoding($true)))
[IO.File]::WriteAllText((Join-Path (Get-Location) 't_utf8_nobom.txt'), $utf8Txt, (New-Object Text.UTF8Encoding($false)))

$write16 = {
  param($name, $enc, $withBom, $text)
  $bytes = $enc.GetBytes($text)
  if ($withBom) { $bytes = $enc.GetPreamble() + $bytes }
  [IO.File]::WriteAllBytes((Join-Path (Get-Location) $name), $bytes)
}
& $write16 't_utf16le.txt'      ([Text.Encoding]::Unicode)   $true  $multi
& $write16 't_utf16le_nobom.txt' ([Text.Encoding]::Unicode)  $false $multi
& $write16 't_utf16be_nobom.txt' ([Text.Encoding]::BigEndianUnicode) $false $multi

$cases = @(
  ,@('t_ascii.txt',           $asciiTxt)
  ,@('t_utf8.txt',            $utf8Txt)
  ,@('t_utf8_nobom.txt',      $utf8Txt)
  ,@('t_utf16le.txt',         $multi)
  ,@('t_utf16le_nobom.txt',   $multi)
  ,@('t_utf16be_nobom.txt',   $multi)
)
$fail = 0
foreach ($c in $cases) {
  $file = Join-Path (Get-Location) $c[0]
  $p = Start-Process -FilePath (Join-Path (Get-Location) 'Notepad.exe') -ArgumentList ('"' + $file + '"') -PassThru
  Start-Sleep -Milliseconds 1600
  $main = Find-MainWindow $p.Id
  if ($main -eq [IntPtr]::Zero) { Write-Output ("{0,-24} DEBUG no window (proc exited={1})" -f $c[0], $p.HasExited) }
  $txt = Get-EditText $main
  if ($txt -eq $null -and $main -ne [IntPtr]::Zero) { Write-Output ("{0,-24} DEBUG window ok but no EDIT child" -f $c[0]) }
  if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }
  $norm = if ($txt -eq $null) { $null } else { $txt.TrimEnd([char]0) }
  $ok = ($norm -ne $null -and $norm -ceq $c[1])
  if (-not $ok) { $fail++ }
  Write-Output ("{0,-24} match={1,-5} got=[{2}]" -f $c[0], $ok, $(if ($norm -eq $null) { '<null>' } else { $norm -replace "`r`n", '\n' }))
}
Write-Output "FAILURES=$fail"
if ($fail -gt 0) { exit 1 }
exit 0
