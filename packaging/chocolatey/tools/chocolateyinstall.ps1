$ErrorActionPreference = 'Stop'

$packageName = 'salam'
$toolsDir    = Split-Path -Parent $MyInvocation.MyCommand.Definition
$version     = '@VERSION@'

$packageArgs = @{
  packageName    = $packageName
  unzipLocation  = $toolsDir
  url            = "https://github.com/SalamLang/Salam/releases/download/v$version/salam-$version-windows-i686.zip"
  url64bit       = "https://github.com/SalamLang/Salam/releases/download/v$version/salam-$version-windows-x86_64.zip"
  checksum       = '@SHA256_WINDOWS_I686@'
  checksumType   = 'sha256'
  checksum64     = '@SHA256_WINDOWS_X86_64@'
  checksumType64 = 'sha256'
}

Install-ChocolateyZipPackage @packageArgs

# salam.exe resolves std/ from the directory it lives in. Point it there
# explicitly so a $env:PATH lookup cannot land on the wrong root.
$dir = if ([Environment]::Is64BitOperatingSystem) {
  Join-Path $toolsDir 'salam-windows'
} else {
  Join-Path $toolsDir 'salam-windows-i686'
}

$exe = Join-Path $dir 'salam.exe'
if (-not (Test-Path $exe)) {
  throw "salam.exe not found at $exe after unpacking"
}

Install-ChocolateyEnvironmentVariable -VariableName 'SALAM_STD' `
  -VariableValue (Join-Path $dir 'std') -VariableType 'Machine'

Install-BinFile -Name 'salam' -Path $exe
