Add-Type -AssemblyName System.Drawing
Add-Type @'
using System; using System.Text; using System.Runtime.InteropServices;
public class SC {
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, EnumProc cb, IntPtr l);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out R r);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr a, int x, int y, int cx, int cy, uint f);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr WindowFromPoint(P p);
  [DllImport("user32.dll", EntryPoint="SendMessage", CharSet=CharSet.Unicode)]
  public static extern IntPtr Send(IntPtr h, uint m, IntPtr w, IntPtr l);
  public static readonly IntPtr TOPMOST = new IntPtr(-1);
  public static readonly IntPtr NOTOPMOST = new IntPtr(-2);
  [StructLayout(LayoutKind.Sequential)] public struct R { public int L,T,Rr,B; }
  [StructLayout(LayoutKind.Sequential)] public struct P { public int X, Y; }
}
'@ -ErrorAction SilentlyContinue

# CRITICAL: without this the harness reads DPI-virtualized coordinates but
# CopyFromScreen reads physical pixels, so every sample lands in the wrong place.
[void][SC]::SetProcessDPIAware()

$dir = Get-Location
$payload = "The quick brown fox jumps over the lazy dog." + "`r`n" +
           "Second line of the sample document." + "`r`n" +
           "Third line - checks wrapping and status bar text." + "`r`n" +
           "Fourth line with a long unbroken string: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
[IO.File]::WriteAllText((Join-Path $dir 'shot.txt'), $payload, (New-Object Text.UTF8Encoding($false)))
$p = Start-Process -FilePath (Join-Path $dir 'Notepad.exe') -ArgumentList ('"' + (Join-Path $dir 'shot.txt') + '"') -PassThru
Start-Sleep -Milliseconds 2500

$script:main = [IntPtr]::Zero
$cb = [SC+EnumProc]{ param($hh, $l)
  $pid2 = [uint32]0
  [void][SC]::GetWindowThreadProcessId($hh, [ref]$pid2)
  if ($pid2 -eq [uint32]$p.Id) {
    $s = New-Object Text.StringBuilder 128
    [void][SC]::GetClassName($hh, $s, 128)
    if ($s.ToString() -eq 'NotepadClass') { $script:main = $hh; return $false }
  }
  return $true
}
[void][SC]::EnumWindows($cb, [IntPtr]::Zero)

# add two more tabs, then return to the first one so the document is shown
[void][SC]::Send($script:main, 0x0111, [IntPtr]3101, [IntPtr]::Zero) ; Start-Sleep -Milliseconds 350
[void][SC]::Send($script:main, 0x0111, [IntPtr]3101, [IntPtr]::Zero) ; Start-Sleep -Milliseconds 350
[void][SC]::Send($script:main, 0x0111, [IntPtr]3403, [IntPtr]::Zero) ; Start-Sleep -Milliseconds 600

$script:status = [IntPtr]::Zero
$cb2 = [SC+EnumProc]{ param($hh, $l)
  $s = New-Object Text.StringBuilder 64
  [void][SC]::GetClassName($hh, $s, 64)
  if ($s.ToString() -eq 'msctls_statusbar32') { $script:status = $hh; return $false }
  return $true
}
[void][SC]::EnumChildWindows($script:main, $cb2, [IntPtr]::Zero)

[void][SC]::SetWindowPos($script:main, [SC]::TOPMOST, 0, 0, 0, 0, 0x0001 -bor 0x0040)
[void][SC]::BringWindowToTop($script:main)
Start-Sleep -Milliseconds 900

$rm = New-Object SC+R
[void][SC]::GetWindowRect($script:main, [ref]$rm)
$st = New-Object SC+R
[void][SC]::GetWindowRect($script:status, [ref]$st)
$w = $rm.Rr - $rm.L; $h = $rm.B - $rm.T
Write-Output ("main  (physical) " + $w + "x" + $h + " at " + $rm.L + "," + $rm.T)
Write-Output ("status(physical) " + ($st.Rr-$st.L) + "x" + ($st.B-$st.T) + " at " + $st.L + "," + $st.T)

$bmp = New-Object Drawing.Bitmap $w, $h
$g = [Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($rm.L, $rm.T, 0, 0, (New-Object Drawing.Size $w, $h))
$g.Dispose()
$bmp.Save((Join-Path $dir 'screenshot.png'), [Drawing.Imaging.ImageFormat]::Png)

function P($x,$y,$label) { $c = $script:bb.GetPixel($x,$y); Write-Output ("  {0,-11} ({1},{2}) #{3:X2}{4:X2}{5:X2}" -f $label,$x,$y,$c.R,$c.G,$c.B) }
$script:bb = $bmp
P ([int]($w/2)) 20                      'titlebar'
P ([int]($w/2)) ([int]($h*0.08))        'tabstrip'
P ([int]($w/2)) ([int]($h*0.45))        'document'
P ($st.L - $rm.L + 400) ($st.T - $rm.T + 10) 'status-mid'
P ($st.L - $rm.L + 60)  ($st.T - $rm.T + 10) 'status-left'

# who is on top at the status bar point?
$pt = New-Object SC+P; $pt.X = $st.L + 400; $pt.Y = $st.T + 10
$wh = [SC]::WindowFromPoint($pt)
$sb = New-Object Text.StringBuilder 64
if ($wh -ne [IntPtr]::Zero) { [void][SC]::GetClassName($wh, $sb, 64) }
Write-Output ("  window at status point = " + $sb.ToString())
$bmp.Dispose()
[void][SC]::SetWindowPos($script:main, [SC]::NOTOPMOST, 0, 0, 0, 0, 0x0001 -bor 0x0002 -bor 0x0040)
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }