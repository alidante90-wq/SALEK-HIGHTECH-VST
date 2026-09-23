# GITI Early Access code generator (Windows PowerShell — no Python needed)
param(
  [Parameter(Mandatory=$true)][string]$MachineId,
  [int]$Days = 0
)
$Pepper = [byte[]](0x53,0x41,0x4c,0x45,0x4b,0x2d,0x47,0x49,0x54,0x49,0x2d,0x45,0x41,0x32,0x30,0x32,0x36,0x2d,0x48,0x54,0x2d,0x9a,0x3c,0x71,0xe2,0x44)
function Normalize([string]$s) {
  ($s.ToUpper() -replace '[^A-Z0-9]', '')
}
function Hash-Hex([string]$s) {
  $data = [System.Text.Encoding]::UTF8.GetBytes($s)
  [uint64]$h = 1469598103934665603
  foreach ($b in $data) {
    $h = $h -bxor $b
    $h = ($h * 1099511628211) -band [uint64]::MaxValue
    foreach ($p in $Pepper) {
      $h = $h -bxor $p
      $h = (($h -shl 7) -bor ($h -shr 57)) -band [uint64]::MaxValue
    }
  }
  $out = ""
  for ($i=0; $i -lt 3; $i++) {
    $part = ($h -shr (16*$i)) -band 0xFFFF
    $out += ("{0:X4}" -f $part)
  }
  return $out
}
$mid = Normalize $MachineId
$payload = "$mid|D$Days|GITI-EA1"
$h = Hash-Hex $payload
Write-Output ("SALEK-{0}-{1}-{2}" -f $h.Substring(0,4), $h.Substring(4,4), $h.Substring(8,4))
