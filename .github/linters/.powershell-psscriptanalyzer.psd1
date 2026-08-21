# PSScriptAnalyzer settings for the Super-Linter PowerShell check.
#
# The only PowerShell in this repository is install.ps1, a console
# installer that has to keep working on a stock Windows 7 box. The two
# rules excluded below are the two that its job requires it to break.
@{
    IncludeDefaultRules = $true

    ExcludeRules        = @(
        # Everything the installer prints is a message for the person
        # watching it work: step headers, a download progress bar,
        # warnings. Write-Output would put all of that on the pipeline,
        # where the functions that return a value would pick it up as
        # part of their result.
        'PSAvoidUsingWriteHost',

        # The empty catch blocks are the compatibility fallbacks: a TLS
        # handshake an old .NET cannot make, a registry key Windows will
        # not open, a .NET 4.5 assembly that is not installed. Failure is
        # the expected branch there, and the next attempt is the handler.
        'PSAvoidUsingEmptyCatchBlock'
    )
}
