$ErrorActionPreference = 'Stop'

Uninstall-BinFile -Name 'salam'
Uninstall-ChocolateyEnvironmentVariable -VariableName 'SALAM_STD' -VariableType 'Machine'
