{
  lib,
  stdenv,
  fetchurl,
  autoPatchelfHook,
  makeWrapper,
  libxml2,
  ncurses,
  zlib,
}:

let
  version = "@VERSION@";

  sources = {
    x86_64-linux = {
      url = "https://github.com/SalamLang/Salam/releases/download/v${version}/salam-${version}-linux-x86_64.tar.gz";
      hash = "@SRI_LINUX_X86_64@";
      dir = "salam-linux-x86_64";
    };
    aarch64-linux = {
      url = "https://github.com/SalamLang/Salam/releases/download/v${version}/salam-${version}-linux-aarch64.tar.gz";
      hash = "@SRI_LINUX_AARCH64@";
      dir = "salam-linux-aarch64";
    };
    aarch64-darwin = {
      url = "https://github.com/SalamLang/Salam/releases/download/v${version}/salam-${version}-macos-arm64.tar.gz";
      hash = "@SRI_MACOS_ARM64@";
      dir = "salam-macos-arm64";
    };
  };

  source =
    sources.${stdenv.hostPlatform.system}
      or (throw "salam: no binary release for ${stdenv.hostPlatform.system}");
in
stdenv.mkDerivation {
  pname = "salam";
  inherit version;

  src = fetchurl { inherit (source) url hash; };

  sourceRoot = source.dir;

  nativeBuildInputs = [ makeWrapper ] ++ lib.optional stdenv.hostPlatform.isLinux autoPatchelfHook;

  buildInputs = lib.optionals stdenv.hostPlatform.isLinux [
    libxml2
    ncurses
    zlib
    stdenv.cc.cc.lib
  ];

  dontConfigure = true;
  dontBuild = true;

  installPhase = ''
    runHook preInstall

    mkdir -p $out/lib/salam
    install -Dm755 salam $out/lib/salam/salam
    cp -r std $out/lib/salam/std
    [ -d sysroots ] && cp -r sysroots $out/lib/salam/sysroots

    # salam finds std/ relative to its own path, which a bare $PATH lookup
    # does not supply, so the wrapper pins it.
    makeWrapper $out/lib/salam/salam $out/bin/salam \
      --set-default SALAM_STD $out/lib/salam/std

    runHook postInstall
  '';

  doInstallCheck = true;
  installCheckPhase = ''
    runHook preInstallCheck

    printf 'func main:\n    println "salam"\nend\n' > hello.salam
    $out/bin/salam exec hello.salam | grep -qx salam

    runHook postInstallCheck
  '';

  meta = {
    description = "General-purpose systems programming language with a built-in DSL";
    homepage = "https://github.com/SalamLang/Salam";
    changelog = "https://github.com/SalamLang/Salam/releases/tag/v${version}";
    license = lib.licenses.gpl3Plus;
    mainProgram = "salam";
    maintainers = with lib.maintainers; [ ];
    platforms = builtins.attrNames sources;
    sourceProvenance = with lib.sourceTypes; [ binaryNativeCode ];
  };
}
