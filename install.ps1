# Salam installer for Windows.
#
# Usage:
#   powershell -NoProfile -ExecutionPolicy Bypass -File install.ps1
#   .\install.ps1 --dir C:\Tools\Salam
#   .\install.ps1 --version 1.2.3
#   .\install.ps1 --no-modify-path
#
# Environment variables:
#   SALAM_INSTALL_DIR      same as --dir
#   SALAM_VERSION          same as --version
#   SALAM_NO_MODIFY_PATH   set to 1 to never touch the user PATH
#
# Default installation directory:
#   %USERPROFILE%\.salam\bin
#
# Release assets this understands:
#   windows        Windows x86_64
#   windows-i686   Windows 32-bit, and ARM through x86 emulation
#
# Written against PowerShell 2.0 so a stock Windows 7 SP1 box can run it
# unchanged: no Invoke-WebRequest, no Expand-Archive, no ConvertFrom-Json,
# no ternaries, and no .NET 4-only method overloads anywhere below. Newer
# Windows just takes the faster path through the same code.

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$RepoOwner = "SalamLang"
$RepoName = "Salam"
$Repo = "$RepoOwner/$RepoName"
$UserAgent = "SalamInstaller/2.0"

$InstallDir = $env:SALAM_INSTALL_DIR
$Version = $env:SALAM_VERSION
$ModifyPath = $true

if ($null -ne $env:SALAM_NO_MODIFY_PATH -and $env:SALAM_NO_MODIFY_PATH -ne "") {
    $ModifyPath = $false
}

# Download methods, best first. A method that fails on transport (rather
# than on a missing file) gets dropped for the rest of the run, so the
# Windows 7 fallback chain is paid for once instead of once per attempt.
$script:Methods = @("net", "bits", "certutil")

$script:StepNo = 0
$script:StepTotal = 7

# ---------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------

$script:LineWidth = 78
$script:CanRewrite = $true

try {
    $width = $Host.UI.RawUI.WindowSize.Width

    if ($width -gt 20) {
        $script:LineWidth = $width - 2
    }
}
catch {
    # No raw UI (redirected output, ISE, remoting): print whole lines
    # instead of repainting one.
    $script:CanRewrite = $false
}

if ("$($Host.Name)" -match "ISE") {
    $script:CanRewrite = $false
}

function Write-Step {
    param([string]$Text)

    $script:StepNo = $script:StepNo + 1

    Write-Host ""
    Write-Host ("[{0}/{1}] {2}" -f $script:StepNo, $script:StepTotal, $Text) `
        -ForegroundColor Cyan
}

function Write-Info {
    param([string]$Text)

    Write-Host "      $Text"
}

function Write-Warn {
    param([string]$Text)

    Write-Host "      warning: $Text" -ForegroundColor Yellow
}

function Write-Status {
    param([string]$Text)

    if ($script:CanRewrite) {
        $pad = $script:LineWidth - $Text.Length

        if ($pad -lt 0) {
            $pad = 0
        }

        Write-Host ("`r" + $Text + (" " * $pad)) -NoNewline
    }
    else {
        Write-Host $Text
    }
}

function Write-StatusEnd {
    if ($script:CanRewrite) {
        Write-Host ""
    }
}

function Fail {
    param([string]$Message)

    Write-Host ""
    Write-Host "error: $Message" -ForegroundColor Red
    Write-Host ""

    exit 1
}

function Show-Usage {
    Write-Host ""
    Write-Host "Salam installer for Windows"
    Write-Host ""
    Write-Host "Usage:"
    Write-Host "  install.ps1 [--dir DIR] [--version X.Y.Z] [--no-modify-path]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  --dir DIR          install into DIR (default %USERPROFILE%\.salam\bin)"
    Write-Host "  --version X.Y.Z    install this exact release instead of the latest"
    Write-Host "  --no-modify-path   never touch the user PATH"
    Write-Host "  -h, --help         show this help"
    Write-Host ""
    Write-Host "Environment:"
    Write-Host "  SALAM_INSTALL_DIR, SALAM_VERSION, SALAM_NO_MODIFY_PATH"
    Write-Host ""
}

function Format-Size {
    param([double]$Bytes)

    if ($Bytes -ge 1073741824) {
        return ("{0:N2} GB" -f ($Bytes / 1073741824))
    }

    if ($Bytes -ge 1048576) {
        return ("{0:N1} MB" -f ($Bytes / 1048576))
    }

    if ($Bytes -ge 1024) {
        return ("{0:N0} KB" -f ($Bytes / 1024))
    }

    return ("{0:N0} B" -f $Bytes)
}

function Format-Duration {
    param([double]$Seconds)

    # A stalled transfer produces absurd estimates; TimeSpan.FromSeconds
    # throws outright past its range, so clamp before formatting.
    if ($Seconds -lt 0 -or $Seconds -gt 359999) {
        return "--:--"
    }

    $total = [int]$Seconds
    $hours = [Math]::Floor($total / 3600)
    $minutes = [Math]::Floor(($total % 3600) / 60)
    $seconds = $total % 60

    if ($hours -gt 0) {
        return ("{0}:{1:00}:{2:00}" -f $hours, $minutes, $seconds)
    }

    return ("{0:00}:{1:00}" -f $minutes, $seconds)
}

# ---------------------------------------------------------------------
# Platform detection
# ---------------------------------------------------------------------

function Enable-Tls12 {
    # github.com has required TLS 1.2 since 2018. .NET 4.5+ knows the
    # value; older runtimes (PowerShell 2.0 on CLR 2.0, notably) throw
    # here and can never reach GitHub over HttpWebRequest at all - that
    # is what the BITS and certutil fallbacks are for, since both go
    # through WinHTTP and use whatever the OS itself can negotiate.
    try {
        # 3072 = Tls12, 768 = Tls11, 192 = Tls. Written numerically
        # because the named enum members do not exist on old runtimes.
        [Net.ServicePointManager]::SecurityProtocol = 3072 -bor 768 -bor 192
        return $true
    }
    catch {
    }

    try {
        [Net.ServicePointManager]::SecurityProtocol = 3072
        return $true
    }
    catch {
    }

    return $false
}

function Get-OsInfo {
    $build = 0
    $major = 0
    $minor = 0
    $product = ""

    # The registry is the honest source: Environment.OSVersion lies about
    # 8.1 and newer unless the calling binary carries a compatibility
    # manifest, and the ARM64 decision below depends on the real build.
    try {
        $key = [Microsoft.Win32.Registry]::LocalMachine.OpenSubKey(
            "SOFTWARE\Microsoft\Windows NT\CurrentVersion")

        if ($null -ne $key) {
            $product = [string]$key.GetValue("ProductName", "")

            $rawBuild = [string]$key.GetValue("CurrentBuildNumber", "")

            if ($rawBuild -match "^\d+$") {
                $build = [int]$rawBuild
            }

            $rawMajor = $key.GetValue("CurrentMajorVersionNumber", $null)

            if ($null -ne $rawMajor) {
                $major = [int]$rawMajor
                $rawMinor = $key.GetValue("CurrentMinorVersionNumber", $null)

                if ($null -ne $rawMinor) {
                    $minor = [int]$rawMinor
                }
            }
            else {
                $rawVersion = [string]$key.GetValue("CurrentVersion", "")

                if ($rawVersion -match "^(\d+)\.(\d+)") {
                    $major = [int]$matches[1]
                    $minor = [int]$matches[2]
                }
            }

            $key.Close()
        }
    }
    catch {
    }

    if ($major -eq 0) {
        try {
            $version = [Environment]::OSVersion.Version
            $major = $version.Major
            $minor = $version.Minor

            if ($build -eq 0) {
                $build = $version.Build
            }
        }
        catch {
        }
    }

    $name = "Windows"

    if ($major -eq 10 -and $build -ge 22000) {
        $name = "Windows 11"
    }
    elseif ($major -eq 10) {
        $name = "Windows 10"
    }
    elseif ($major -eq 6 -and $minor -eq 3) {
        $name = "Windows 8.1"
    }
    elseif ($major -eq 6 -and $minor -eq 2) {
        $name = "Windows 8"
    }
    elseif ($major -eq 6 -and $minor -eq 1) {
        $name = "Windows 7"
    }
    elseif ($major -eq 6 -and $minor -eq 0) {
        $name = "Windows Vista"
    }
    elseif ($product -ne "") {
        $name = $product
    }

    if ($product -ne "" -and $product -match "Server") {
        $name = $product
    }

    $info = New-Object PSObject
    $info | Add-Member NoteProperty Name $name
    $info | Add-Member NoteProperty Major $major
    $info | Add-Member NoteProperty Minor $minor
    $info | Add-Member NoteProperty Build $build

    return $info
}

function Get-Platform {
    param($OsInfo)

    $arch = $env:PROCESSOR_ARCHITECTURE

    # Set only inside a 32-bit process on a 64-bit Windows, where
    # PROCESSOR_ARCHITECTURE reads x86 no matter what the machine is.
    if ($null -ne $env:PROCESSOR_ARCHITEW6432 -and
        $env:PROCESSOR_ARCHITEW6432 -ne "") {
        $arch = $env:PROCESSOR_ARCHITEW6432
    }

    if ($arch -ieq "AMD64" -or $arch -ieq "x64" -or $arch -ieq "EM64T") {
        return "windows"
    }

    if ($arch -ieq "x86") {
        return "windows-i686"
    }

    if ($arch -ieq "ARM64") {
        # Windows 11 on ARM emulates x64; Windows 10 on ARM only ever
        # emulated x86, so it gets the 32-bit build.
        if ($OsInfo.Build -ge 22000) {
            return "windows"
        }

        return "windows-i686"
    }

    if ($arch -ieq "ARM") {
        Fail ("32-bit ARM Windows cannot run either published Salam build. " +
            "Build from source: https://github.com/$Repo")
    }

    Fail ("unsupported processor architecture: $arch. Salam publishes " +
        "windows (x86_64) and windows-i686 (32-bit) builds only.")
}

# ---------------------------------------------------------------------
# Downloading
# ---------------------------------------------------------------------

function Invoke-NetDownload {
    param(
        [string]$Url,
        [string]$Destination,
        [bool]$ShowProgress
    )

    $request = $null
    $response = $null
    $inStream = $null
    $output = $null

    try {
        $request = [Net.HttpWebRequest]::Create($Url)
        $request.Method = "GET"
        $request.AllowAutoRedirect = $true
        $request.UserAgent = $UserAgent
        $request.Timeout = 60000

        try {
            $request.ReadWriteTimeout = 120000
        }
        catch {
        }

        # Corporate proxies configured in Internet Options usually want
        # the caller's own credentials; without this they answer 407.
        try {
            if ($null -ne $request.Proxy) {
                $request.Proxy.Credentials =
                    [Net.CredentialCache]::DefaultCredentials
            }
        }
        catch {
        }
    }
    catch {
        return "error: " + $_.Exception.Message
    }

    try {
        $response = $request.GetResponse()
    }
    catch [Net.WebException] {
        $webResponse = $_.Exception.Response

        if ($null -ne $webResponse) {
            $status = [int]$webResponse.StatusCode
            $webResponse.Close()

            if ($status -eq 404) {
                return "notfound"
            }

            return "error: HTTP $status"
        }

        return "error: " + $_.Exception.Message
    }
    catch {
        return "error: " + $_.Exception.Message
    }

    try {
        $inStream = $response.GetResponseStream()

        $output = New-Object System.IO.FileStream(
            $Destination,
            [System.IO.FileMode]::Create,
            [System.IO.FileAccess]::Write,
            [System.IO.FileShare]::None)

        $total = $response.ContentLength

        if ($total -lt 0) {
            $total = 0
        }

        $buffer = New-Object byte[] 65536
        $done = [long]0
        $start = [DateTime]::UtcNow
        $lastPaint = [DateTime]::UtcNow
        $lastPercent = -1

        while ($true) {
            $read = $inStream.Read($buffer, 0, $buffer.Length)

            if ($read -le 0) {
                break
            }

            $output.Write($buffer, 0, $read)
            $done = $done + $read

            if (!$ShowProgress) {
                continue
            }

            $now = [DateTime]::UtcNow
            $paint = ($now - $lastPaint).TotalMilliseconds -ge 150

            if ($total -gt 0 -and !$script:CanRewrite) {
                # Without cursor control every repaint is a new line, so
                # only speak up once per 10% instead of 6 times a second.
                $percent = [int](($done * 100) / $total)
                $paint = ($percent -ge $lastPercent + 10)

                if ($paint) {
                    $lastPercent = $percent - ($percent % 10)
                }
            }

            if ($paint) {
                Write-Status (Format-ProgressLine -Done $done -Total $total -Start $start)
                $lastPaint = $now
            }
        }

        if ($ShowProgress) {
            Write-Status (Format-ProgressLine -Done $done -Total $total -Start $start)
            Write-StatusEnd
        }

        return "ok"
    }
    catch {
        return "error: " + $_.Exception.Message
    }
    finally {
        if ($null -ne $inStream) {
            $inStream.Close()
        }

        if ($null -ne $output) {
            $output.Close()
        }

        if ($null -ne $response) {
            $response.Close()
        }
    }
}

function Format-ProgressLine {
    param(
        [long]$Done,
        [long]$Total,
        $Start
    )

    $elapsed = ([DateTime]::UtcNow - $Start).TotalSeconds

    if ($elapsed -le 0) {
        $elapsed = 0.001
    }

    $speed = $Done / $elapsed
    $rate = (Format-Size $speed) + "/s"

    if ($Total -le 0) {
        return ("      {0}  {1}" -f (Format-Size $Done), $rate)
    }

    $percent = ($Done * 100.0) / $Total
    $slots = 24
    $filled = [int][Math]::Floor(($percent / 100.0) * $slots)

    if ($filled -gt $slots) {
        $filled = $slots
    }

    if ($filled -lt 0) {
        $filled = 0
    }

    $bar = ("#" * $filled) + ("." * ($slots - $filled))

    $eta = "--:--"

    if ($speed -gt 0) {
        $eta = Format-Duration (($Total - $Done) / $speed)
    }

    return ("      [{0}] {1,3:N0}%  {2} / {3}  {4}  ETA {5}" -f `
            $bar, $percent, (Format-Size $Done), (Format-Size $Total), $rate, $eta)
}

function Invoke-BitsDownload {
    param(
        [string]$Url,
        [string]$Destination
    )

    try {
        Import-Module BitsTransfer -ErrorAction Stop

        Start-BitsTransfer -Source $Url -Destination $Destination `
            -Description "Salam" -ErrorAction Stop

        if (Test-Path $Destination) {
            return "ok"
        }

        return "error: BITS wrote no file"
    }
    catch {
        $message = $_.Exception.Message

        if ("$message" -match "404") {
            return "notfound"
        }

        return "error: " + $message
    }
}

function Invoke-CertUtilDownload {
    param(
        [string]$Url,
        [string]$Destination
    )

    # certutil.exe has shipped in the box since Vista and downloads over
    # WinHTTP, so it works on machines whose .NET is too old to speak
    # TLS 1.2 - the usual Windows 7 case.
    $ErrorActionPreference = "Continue"

    $certutil = Join-Path $env:SystemRoot "System32\certutil.exe"

    if (!(Test-Path $certutil)) {
        return "error: certutil.exe not found"
    }

    try {
        $null = & $certutil -urlcache -split -f $Url $Destination 2>&1
        $code = $LASTEXITCODE

        # The URL cache keeps a copy of everything it fetches; drop it so
        # a reinstall does not silently reuse a stale archive.
        $null = & $certutil -urlcache -f $Url delete 2>&1

        if ($code -eq 0 -and (Test-Path $Destination)) {
            return "ok"
        }

        if ($code -eq 0) {
            return "error: certutil wrote no file"
        }

        return "error: certutil exit code $code"
    }
    catch {
        return "error: " + $_.Exception.Message
    }
}

function Save-Url {
    param(
        [string]$Url,
        [string]$Destination,
        [bool]$ShowProgress,
        [bool]$Quiet
    )

    if (@($script:Methods).Count -eq 0) {
        Fail ("this machine cannot download over HTTPS - PowerShell, BITS and " +
            "certutil all failed. Fetch a release archive by hand from " +
            "https://github.com/$Repo/releases")
    }

    $errors = @()

    foreach ($method in @($script:Methods)) {
        if (Test-Path $Destination) {
            Remove-Item $Destination -Force -ErrorAction SilentlyContinue
        }

        if ($method -eq "net") {
            $result = Invoke-NetDownload -Url $Url -Destination $Destination -ShowProgress $ShowProgress
        }
        elseif ($method -eq "bits") {
            if (!$Quiet) {
                Write-Info "trying BITS..."
            }

            $result = Invoke-BitsDownload -Url $Url -Destination $Destination
        }
        else {
            if (!$Quiet) {
                Write-Info "trying certutil..."
            }

            $result = Invoke-CertUtilDownload -Url $Url -Destination $Destination
        }

        if ($result -eq "ok") {
            # Whatever worked goes first from now on.
            $rest = @($script:Methods | Where-Object { $_ -ne $method })
            $script:Methods = @($method) + $rest

            return $true
        }

        if ($result -eq "notfound") {
            # A definite answer from the server: no other transport will
            # find this file either.
            return $false
        }

        $detail = "$result"

        if ($detail.StartsWith("error: ")) {
            $detail = $detail.Substring(7)
        }

        $errors += "$method - $detail"

        # An old runtime that cannot negotiate TLS 1.2 fails this way
        # every single time, so stop paying for it on later attempts.
        if ("$result" -match "SSL|secure channel|TLS|trust relationship|Authentication") {
            $script:Methods = @($script:Methods | Where-Object { $_ -ne $method })
        }
    }

    if (!$Quiet) {
        foreach ($message in $errors) {
            Write-Warn $message
        }
    }

    return $false
}

function Get-UrlText {
    param([string]$Url)

    $temp = Join-Path $script:WorkDir ("fetch-" + [Guid]::NewGuid().ToString() + ".txt")

    if (!(Save-Url -Url $Url -Destination $temp -ShowProgress $false -Quiet $true)) {
        return ""
    }

    try {
        $text = [IO.File]::ReadAllText($temp)
    }
    catch {
        $text = ""
    }

    Remove-Item $temp -Force -ErrorAction SilentlyContinue

    return $text
}

# ---------------------------------------------------------------------
# Release lookup
# ---------------------------------------------------------------------

function Get-RedirectedLatestTag {
    # /releases/latest answers with a redirect to the real tag and costs
    # no API quota, unlike api.github.com which rate-limits per IP.
    try {
        $request = [Net.HttpWebRequest]::Create(
            "https://github.com/$Repo/releases/latest")
        $request.Method = "HEAD"
        $request.AllowAutoRedirect = $false
        $request.UserAgent = $UserAgent
        $request.Timeout = 30000

        try {
            if ($null -ne $request.Proxy) {
                $request.Proxy.Credentials =
                    [Net.CredentialCache]::DefaultCredentials
            }
        }
        catch {
        }

        $response = $null

        try {
            $response = $request.GetResponse()
        }
        catch [Net.WebException] {
            $response = $_.Exception.Response
        }

        if ($null -eq $response) {
            return ""
        }

        $location = [string]$response.Headers["Location"]
        $response.Close()

        $match = [Regex]::Match($location, "/releases/tag/([^/?#]+)")

        if ($match.Success) {
            return $match.Groups[1].Value
        }
    }
    catch {
    }

    return ""
}

function Get-TagsFromAtom {
    # The Atom feed is served by github.com itself and carries no rate
    # limit, unlike the REST API which answers 403 to busy shared IPs.
    $text = Get-UrlText "https://github.com/$Repo/releases.atom"

    if ($text -eq "") {
        return @()
    }

    $tags = @()

    foreach ($match in [Regex]::Matches($text, "/releases/tag/([^""<]+)")) {
        $tags += $match.Groups[1].Value
    }

    return $tags
}

function Get-TagsFromApi {
    $text = Get-UrlText "https://api.github.com/repos/$Repo/releases?per_page=10"

    if ($text -eq "") {
        return @()
    }

    $tags = @()

    foreach ($match in [Regex]::Matches($text, '"tag_name"\s*:\s*"([^"]+)"')) {
        $tags += $match.Groups[1].Value
    }

    return $tags
}

function Get-CandidateTagList {
    $tags = @()

    $latest = Get-RedirectedLatestTag

    if ($latest -ne "") {
        $tags += $latest
    }

    $tags += Get-TagsFromAtom

    if ($tags.Length -eq 0) {
        $tags += Get-TagsFromApi
    }

    $seen = @{}
    $result = @()

    foreach ($tag in $tags) {
        $clean = "$tag".Trim()

        if ($clean -eq "") {
            continue
        }

        # Nightlies are prereleases whose assets carry the plain version
        # (salam-0.2.9-windows.zip under a v0.2.9-nightly-<date> tag), so
        # the name built from the tag can never match one - skipping them
        # here just avoids a guaranteed 404.
        if ($clean -match "nightly") {
            continue
        }

        $key = $clean.ToLower()

        if ($seen.ContainsKey($key)) {
            continue
        }

        $seen[$key] = $true
        $result += $clean

        # Ten is already more history than any real lookup needs; the
        # loop that tries them pays a round trip per entry.
        if ($result.Length -ge 10) {
            break
        }
    }

    return $result
}

function Test-ZipFile {
    param([string]$Path)

    try {
        $stream = New-Object System.IO.FileStream(
            $Path,
            [System.IO.FileMode]::Open,
            [System.IO.FileAccess]::Read,
            [System.IO.FileShare]::Read)

        $head = New-Object byte[] 2
        $read = $stream.Read($head, 0, 2)
        $stream.Close()

        # "PK". An HTML error page saved under a .zip name fails here
        # instead of turning into a baffling extraction error later.
        return ($read -eq 2 -and $head[0] -eq 80 -and $head[1] -eq 75)
    }
    catch {
        return $false
    }
}

# ---------------------------------------------------------------------
# Extraction
# ---------------------------------------------------------------------

function Expand-ZipArchive {
    param(
        [string]$Archive,
        [string]$Destination
    )

    if (Test-Path $Destination) {
        Remove-Item $Destination -Recurse -Force
    }

    $null = New-Item -ItemType Directory -Path $Destination -Force

    if (Expand-WithDotNet -Archive $Archive -Destination $Destination) {
        return
    }

    Expand-WithShell -Archive $Archive -Destination $Destination

    if (@(Get-ChildItem $Destination -Force -ErrorAction SilentlyContinue).Count -eq 0) {
        Fail "the archive extracted to nothing"
    }
}

function Expand-WithDotNet {
    param(
        [string]$Archive,
        [string]$Destination
    )

    $zip = $null

    # ZipFile arrived with .NET 4.5, and PowerShell 2.0 runs on CLR 2.0
    # even where 4.5 is installed - so this throws on an untouched
    # Windows 7 and the caller falls back to Explorer.
    try {
        Add-Type -AssemblyName System.IO.Compression.FileSystem

        $zip = [System.IO.Compression.ZipFile]::OpenRead($Archive)
    }
    catch {
        return $false
    }

    try {
        $entries = @($zip.Entries)
        $total = $entries.Count
        $done = 0
        $written = 0

        $separator = [string][IO.Path]::DirectorySeparatorChar
        $root = [IO.Path]::GetFullPath($Destination).TrimEnd($separator) + $separator

        # Repaint about 25 times over the whole archive, whatever its
        # size, instead of once per fixed number of files.
        $every = [int][Math]::Ceiling($total / 25.0)

        if ($every -lt 1) {
            $every = 1
        }

        if (!$script:CanRewrite) {
            $every = $every * 5
        }

        foreach ($entry in $entries) {
            $done = $done + 1

            $target = [IO.Path]::GetFullPath(
                (Join-Path $Destination $entry.FullName))

            # An archive is free to name ..\..\somewhere; refuse to
            # write anywhere but the directory we were handed.
            if (!$target.StartsWith($root, [StringComparison]::OrdinalIgnoreCase)) {
                continue
            }

            if ($entry.Name -eq "") {
                $null = New-Item -ItemType Directory -Path $target -Force
                continue
            }

            $parent = [IO.Path]::GetDirectoryName($target)

            if (!(Test-Path $parent)) {
                $null = New-Item -ItemType Directory -Path $parent -Force
            }

            [System.IO.Compression.ZipFileExtensions]::ExtractToFile(
                $entry, $target, $true)

            $written = $written + 1

            if (($done % $every) -eq 0 -or $done -eq $total) {
                Write-Status ("      {0,3:N0}%  {1} / {2} files" -f `
                    (($done * 100.0) / $total), $done, $total)
            }
        }

        Write-StatusEnd

        # Nothing written means this path did not really work; let the
        # caller try Explorer rather than reporting a hollow success.
        return ($written -gt 0)
    }
    catch {
        Fail "could not extract the archive: $($_.Exception.Message)"
    }
    finally {
        if ($null -ne $zip) {
            $zip.Dispose()
        }
    }
}

function Expand-WithShell {
    param(
        [string]$Archive,
        [string]$Destination
    )

    try {
        $shell = New-Object -ComObject Shell.Application

        $zipFolder = $shell.NameSpace($Archive)
        $destFolder = $shell.NameSpace($Destination)

        if ($null -eq $zipFolder -or $null -eq $destFolder) {
            Fail "could not open the downloaded archive"
        }

        # 4 = no progress dialog, 16 = yes to all, 512 = do not ask about
        # creating folders, 1024 = no error dialog.
        $destFolder.CopyHere($zipFolder.Items(), 1556)

        # CopyHere returns as soon as the copy is queued and the archive
        # root shows up on disk immediately, so counting entries proves
        # nothing. Watch the tree grow instead and call it finished only
        # once it stops growing.
        $waited = 0
        $stalled = 0
        $previous = ""

        while ($true) {
            Start-Sleep -Milliseconds 2000
            $waited = $waited + 2000

            $files = @(Get-ChildItem $Destination -Recurse -Force `
                    -ErrorAction SilentlyContinue)

            $bytes = [long]0

            foreach ($file in $files) {
                if (!$file.PSIsContainer) {
                    $bytes = $bytes + $file.Length
                }
            }

            Write-Status ("      {0} files, {1}" -f $files.Count, (Format-Size $bytes))

            $signature = "$($files.Count):$bytes"

            if ($signature -eq $previous) {
                $stalled = $stalled + 2000
            }
            else {
                $stalled = 0
                $previous = $signature
            }

            if ($files.Count -eq 0) {
                if ($waited -ge 60000) {
                    Write-StatusEnd
                    Fail "Windows extracted nothing from the archive"
                }

                continue
            }

            if ($stalled -ge 16000) {
                break
            }

            if ($waited -ge 1800000) {
                Write-StatusEnd
                Fail "extraction timed out"
            }
        }

        Write-StatusEnd
    }
    catch {
        Fail "could not extract the archive: $($_.Exception.Message)"
    }
}

# ---------------------------------------------------------------------
# PATH
# ---------------------------------------------------------------------

function Publish-EnvironmentChange {
    # Tell already-running shells and Explorer that the environment
    # moved; without it, a new terminal only picks the change up after a
    # logoff on some Windows versions.
    try {
        $signature = @'
[DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
public static extern IntPtr SendMessageTimeout(
    IntPtr hWnd, uint Msg, UIntPtr wParam, string lParam,
    uint fuFlags, uint uTimeout, out UIntPtr lpdwResult);
'@

        $native = Add-Type -MemberDefinition $signature -Name "SalamNative" `
            -Namespace "SalamInstaller" -PassThru -ErrorAction Stop

        $result = [UIntPtr]::Zero

        # HWND_BROADCAST, WM_SETTINGCHANGE, SMTO_ABORTIFHUNG, 5s.
        $null = $native::SendMessageTimeout(
            [IntPtr]0xFFFF, 0x001A, [UIntPtr]::Zero, "Environment",
            2, 5000, [ref]$result)
    }
    catch {
    }
}

function Add-BinDirToPath {
    param([string]$BinDir)

    $key = $null

    try {
        $key = [Microsoft.Win32.Registry]::CurrentUser.CreateSubKey("Environment")

        # DoNotExpandEnvironmentNames keeps entries like %JAVA_HOME%\bin
        # written as-is; reading them expanded and writing them back
        # would bake today's value into the user's PATH forever. setx is
        # avoided for the same reason, plus its 1024-character truncation.
        $current = $key.GetValue(
            "Path", "",
            [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames)

        if ($null -eq $current) {
            $current = ""
        }

        $current = [string]$current
        $target = $BinDir.TrimEnd("\")
        $present = $false

        foreach ($entry in ($current -split ";")) {
            if ($entry.Trim().TrimEnd("\") -ieq $target) {
                $present = $true
                break
            }
        }

        if ($present) {
            Write-Info "already on your user PATH"
        }
        else {
            if ($current.Trim() -eq "") {
                $updated = $BinDir
            }
            else {
                $updated = $current.TrimEnd(";") + ";" + $BinDir
            }

            $kind = [Microsoft.Win32.RegistryValueKind]::ExpandString

            try {
                if ($current -ne "") {
                    $kind = $key.GetValueKind("Path")
                }
            }
            catch {
            }

            $key.SetValue("Path", $updated, $kind)

            Write-Info "added $BinDir to your user PATH"

            Publish-EnvironmentChange
        }

        # Make it usable in this session too, so the verification step
        # below and anything the user runs next already sees it.
        $processPath = "$env:PATH"

        if (@($processPath -split ";" | Where-Object {
                    $_.Trim().TrimEnd("\") -ieq $target
                }).Count -eq 0) {
            $env:PATH = $processPath.TrimEnd(";") + ";" + $BinDir
        }
    }
    catch {
        Write-Warn "could not update PATH automatically: $($_.Exception.Message)"
        Write-Info "add this directory to PATH by hand: $BinDir"
    }
    finally {
        if ($null -ne $key) {
            $key.Close()
        }
    }
}

# ---------------------------------------------------------------------
# Arguments
# ---------------------------------------------------------------------

$index = 0

while ($index -lt $args.Length) {
    $arg = [string]$args[$index]

    if ($arg -ieq "--dir" -or $arg -ieq "-d") {
        if (($index + 1) -ge $args.Length) {
            Fail "$arg requires a directory"
        }

        $InstallDir = [string]$args[$index + 1]
        $index = $index + 2
        continue
    }

    if ($arg -ilike "--dir=*") {
        $InstallDir = $arg.Substring(6)
        $index = $index + 1
        continue
    }

    if ($arg -ieq "--version" -or $arg -ieq "-v") {
        if (($index + 1) -ge $args.Length) {
            Fail "$arg requires a version"
        }

        $Version = [string]$args[$index + 1]
        $index = $index + 2
        continue
    }

    if ($arg -ilike "--version=*") {
        $Version = $arg.Substring(10)
        $index = $index + 1
        continue
    }

    if ($arg -ieq "--no-modify-path") {
        $ModifyPath = $false
        $index = $index + 1
        continue
    }

    if ($arg -ieq "-h" -or $arg -ieq "--help" -or $arg -ieq "/?") {
        Show-Usage
        exit 0
    }

    Show-Usage
    Fail "unknown option: $arg"
}

# ---------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------

Write-Host ""
Write-Host "  Salam Programming Language installer" -ForegroundColor Green
Write-Host "  https://github.com/$Repo"
Write-Host "  ------------------------------------"

Write-Step "Checking this machine"

$os = Get-OsInfo
$platform = Get-Platform -OsInfo $os

$osLabel = $os.Name

if ($os.Build -gt 0) {
    $osLabel = "$osLabel (build $($os.Build))"
}

Write-Info ("windows    : " + $osLabel)
Write-Info ("powershell : " + $PSVersionTable.PSVersion)
Write-Info ("target     : " + $platform)

if (!(Enable-Tls12)) {
    Write-Warn "this PowerShell cannot negotiate TLS 1.2; falling back to BITS/certutil"

    # No point spending a doomed .NET attempt on every request.
    $script:Methods = @($script:Methods | Where-Object { $_ -ne "net" })
}

$homeDir = $env:USERPROFILE

if ($null -eq $homeDir -or $homeDir -eq "") {
    $homeDir = "$env:HOMEDRIVE$env:HOMEPATH"
}

if ($null -eq $homeDir -or $homeDir -eq "") {
    $homeDir = "$env:SystemDrive\"
}

$defaultDir = Join-Path $homeDir ".salam\bin"

if ($null -eq $InstallDir -or "$InstallDir".Trim() -eq "") {
    $InstallDir = $defaultDir
}

try {
    if (![IO.Path]::IsPathRooted($InstallDir)) {
        # Not [IO.Path]::GetFullPath alone: .NET resolves against the
        # process directory, which stopped tracking the shell the first
        # time anyone ran Set-Location.
        $here = (Get-Location).Path

        try {
            $here = (Get-Location).ProviderPath
        }
        catch {
        }

        $InstallDir = Join-Path $here $InstallDir
    }

    $InstallDir = [IO.Path]::GetFullPath($InstallDir)
}
catch {
    Fail "invalid installation directory: $InstallDir"
}

$isDefaultDir = ($InstallDir.TrimEnd("\") -ieq $defaultDir.TrimEnd("\"))

$tempRoot = $env:TEMP

if ($null -eq $tempRoot -or "$tempRoot".Trim() -eq "") {
    $tempRoot = $env:TMP
}

if ($null -eq $tempRoot -or "$tempRoot".Trim() -eq "") {
    $tempRoot = $homeDir
}

$script:WorkDir = Join-Path $tempRoot `
    ("salam-install-" + [Guid]::NewGuid().ToString("N"))

$archive = Join-Path $script:WorkDir "salam.zip"
$extractDir = Join-Path $script:WorkDir "extracted"

try {
    $null = New-Item -ItemType Directory -Path $script:WorkDir -Force

    Write-Step "Choosing the release to install"

    $downloaded = $false
    $asset = ""
    $url = ""

    if ($null -ne $Version -and "$Version".Trim() -ne "") {
        $Version = "$Version".Trim()

        if ($Version.StartsWith("v")) {
            $Version = $Version.Substring(1)
        }

        Write-Info "requested version: $Version"

        $asset = "salam-$Version-$platform.zip"
        $url = "https://github.com/$Repo/releases/download/v$Version/$asset"

        Write-Step "Downloading $asset"
        Write-Info $url

        $downloaded = Save-Url -Url $url -Destination $archive -ShowProgress $true -Quiet $false

        if (!$downloaded) {
            Fail ("release v$Version publishes no $platform asset. " +
                "See https://github.com/$Repo/releases")
        }
    }
    else {
        Write-Info "asking GitHub for the newest release with a $platform asset"

        $tags = @(Get-CandidateTagList)

        if ($tags.Length -eq 0) {
            Fail ("could not reach GitHub to list releases. Check your " +
                "internet connection or pass --version X.Y.Z to skip the lookup.")
        }

        $shown = $tags

        if ($shown.Length -gt 4) {
            $shown = $shown[0..3] + @("...")
        }

        Write-Info ("newest releases: " + ($shown -join ", "))

        $stepAnnounced = $false

        foreach ($tag in $tags) {
            $candidate = $tag

            if ($candidate.StartsWith("v")) {
                $candidate = $candidate.Substring(1)
            }

            $asset = "salam-$candidate-$platform.zip"
            $url = "https://github.com/$Repo/releases/download/$tag/$asset"

            if (!$stepAnnounced) {
                # Named generically: which asset this ends up being is
                # only settled once one of the candidates answers.
                Write-Step "Downloading the release archive"
                $stepAnnounced = $true
            }

            Write-Info $url

            if (Save-Url -Url $url -Destination $archive -ShowProgress $true -Quiet $false) {
                $Version = $candidate
                $downloaded = $true
                break
            }

            Write-Info "$tag has no $platform asset, trying the one before it"
        }

        if (!$downloaded) {
            Fail ("no recent release publishes a $platform asset. " +
                "See https://github.com/$Repo/releases")
        }
    }

    if (!(Test-Path $archive)) {
        Fail "the download produced no file"
    }

    $size = (Get-Item $archive).Length

    if ($size -lt 1024 -or !(Test-ZipFile $archive)) {
        Fail "the downloaded file is not a zip archive - try again"
    }

    Write-Step "Extracting salam-$Version-$platform.zip"
    Write-Info ("archive size: " + (Format-Size $size))

    Expand-ZipArchive -Archive $archive -Destination $extractDir

    Write-Info "extracted"

    Write-Step "Installing into $InstallDir"

    $bundle = Join-Path $extractDir "salam-$platform"

    if (!(Test-Path (Join-Path $bundle "salam.exe"))) {
        $found = @(Get-ChildItem -Path $extractDir -Filter "salam.exe" `
                -Recurse -Force -ErrorAction SilentlyContinue)

        if ($found.Length -eq 0) {
            Fail "salam.exe is missing from $asset"
        }

        $bundle = $found[0].DirectoryName
    }

    $null = New-Item -ItemType Directory -Path $InstallDir -Force

    # Everything ships side by side on purpose: salam.exe locates its
    # std\ library and the bundled tcc\ toolchain relative to its own
    # path, so installing the bare exe leaves every "import os" failing
    # with "standard library package not found". Copying the whole
    # bundle also keeps this script working when a release starts
    # shipping something new next to the compiler.
    foreach ($entry in @(Get-ChildItem -Path $bundle -Force)) {
        $target = Join-Path $InstallDir $entry.Name

        $label = $entry.Name

        if ($entry.PSIsContainer) {
            $label = $label + "\"
        }

        # Announced before the copy rather than after it: tcc\ and std\
        # together are most of a 100 MB bundle and the copy is the one
        # part of this step long enough to look like a hang.
        Write-Info "copying $label"

        try {
            if ($entry.PSIsContainer) {
                if (Test-Path $target) {
                    # Replace wholesale: an upgrade must not leave a
                    # previous version's std\ modules lying around.
                    Remove-Item $target -Recurse -Force
                }

                Copy-Item $entry.FullName $target -Recurse -Force
            }
            else {
                Copy-Item $entry.FullName $target -Force
            }
        }
        catch {
            if ($entry.Name -ieq "salam.exe") {
                Fail ("could not write $target - " +
                    "close any running salam.exe (and any terminal using it) " +
                    "and run the installer again")
            }

            Write-Warn "could not copy $($entry.Name): $($_.Exception.Message)"
        }
    }

    if (!(Test-Path (Join-Path $InstallDir "std"))) {
        Write-Warn "no std\ directory in this release - imports will not resolve"
    }

    if ($ModifyPath -and $isDefaultDir) {
        Write-Step "Updating your PATH"

        Add-BinDirToPath -BinDir $InstallDir
    }
    elseif ($ModifyPath) {
        Write-Step "Leaving PATH alone"

        Write-Info "PATH is only updated for the default directory"
        Write-Info "add this by hand if you want salam on PATH: $InstallDir"
    }
    else {
        Write-Step "Leaving PATH alone"

        Write-Info "--no-modify-path was given"
    }

    Write-Step "Verifying the installation"

    $exe = Join-Path $InstallDir "salam.exe"

    if (!(Test-Path $exe)) {
        Fail "salam.exe is not at $exe after installing"
    }

    Write-Info $exe

    try {
        # A native command writing to stderr becomes a terminating error
        # under the script's Stop preference, so relax it just here.
        $ErrorActionPreference = "Continue"

        & $exe version
    }
    catch {
        # Only the first line: PowerShell appends the offending script
        # line and a caret to native failures, which says nothing here.
        $reason = ("$($_.Exception.Message)" -split "`n")[0]

        Write-Warn "could not run salam.exe: $reason"
    }

    $ErrorActionPreference = "Stop"

    Write-Host ""
    Write-Host "  Salam $Version is installed." -ForegroundColor Green
    Write-Host ""
    Write-Host "  Open a new terminal and try:"
    Write-Host ""
    Write-Host "      salam version"
    Write-Host "      salam run hello.salam"
    Write-Host ""
}
catch {
    Write-Host ""
    Write-Host "error: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""

    exit 1
}
finally {
    if ($null -ne $script:WorkDir -and (Test-Path $script:WorkDir)) {
        Remove-Item $script:WorkDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

exit 0
