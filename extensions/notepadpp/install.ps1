$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$npp = Join-Path $env:APPDATA "Notepad++"
foreach ($d in "userDefineLangs", "autoCompletion", "functionList") {
    New-Item -ItemType Directory -Force -Path (Join-Path $npp $d) | Out-Null
}
Copy-Item (Join-Path $here "salam.xml") (Join-Path $npp "userDefineLangs\salam.xml") -Force
Copy-Item (Join-Path $here "salam-dark.xml") (Join-Path $npp "userDefineLangs\salam-dark.xml") -Force
Copy-Item (Join-Path $here "autoCompletion\Salam.xml") (Join-Path $npp "autoCompletion\Salam.xml") -Force
Copy-Item (Join-Path $here "autoCompletion\Salam.xml") (Join-Path $npp "autoCompletion\Salam Dark.xml") -Force
Copy-Item (Join-Path $here "functionList\salam.xml") (Join-Path $npp "functionList\salam.xml") -Force
Write-Host "Copied. Now add the associations to functionList\overrideMap.xml and merge shortcuts-fragment.xml; see README.md."
