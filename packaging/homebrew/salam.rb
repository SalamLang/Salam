# frozen_string_literal: true

class Salam < Formula
  desc "General-purpose systems programming language with a built-in DSL"
  homepage "https://github.com/SalamLang/Salam"
  url "https://github.com/SalamLang/Salam/archive/refs/tags/v@VERSION@.tar.gz"
  sha256 "@SHA256_SRC@"
  license "GPL-3.0-or-later"
  head "https://github.com/SalamLang/Salam.git", branch: "main"

  depends_on "llvm@22" => :build
  depends_on "libxml2"
  depends_on "zstd"

  uses_from_macos "zlib"

  on_macos do
    # Upstream publishes no Intel macOS release, so there is no seed compiler
    # to bootstrap from on x86_64.
    depends_on arch: :arm64
  end

  # The Salam compiler is written in Salam, so building it needs an existing
  # Salam. This is the previous tagged release, used only to compile the
  # in-tree compiler; nothing from it is installed.
  resource "bootstrap" do
    on_macos do
      url "https://github.com/SalamLang/Salam/releases/download/v@SEED_VERSION@/salam-@SEED_VERSION@-macos-arm64.tar.gz"
      sha256 "@SHA256_SEED_MACOS_ARM64@"
    end
    on_linux do
      on_intel do
        url "https://github.com/SalamLang/Salam/releases/download/v@SEED_VERSION@/salam-@SEED_VERSION@-linux-x86_64.tar.gz"
        sha256 "@SHA256_SEED_LINUX_X86_64@"
      end
      on_arm do
        url "https://github.com/SalamLang/Salam/releases/download/v@SEED_VERSION@/salam-@SEED_VERSION@-linux-aarch64.tar.gz"
        sha256 "@SHA256_SEED_LINUX_AARCH64@"
      end
    end
  end

  def install
    seed = buildpath / "bootstrap"
    resource("bootstrap").stage do
      seed.install Dir["*"]
    end

    # libsalam_llvm.a merges the native shims with LLVM's static archives, so
    # the compiler carries its own LLVM backend instead of shelling out.
    system "sh", "std/llvm/native/build.sh",
           "--llvm-config", formula_opt_bin("llvm@22") / "llvm-config",
           "--out", buildpath

    ENV.prepend_path "LIBRARY_PATH", formula_opt_lib("zstd")
    ENV.prepend_path "LIBRARY_PATH", formula_opt_lib("libxml2")
    ENV.prepend_path "CPATH", formula_opt_include("libxml2") / "libxml2"

    system "sh", "tools/bash/build-selfhost.sh",
           "--output=#{buildpath}/salam",
           "--seed=#{seed}/salam",
           "--llvm=#{buildpath}"

    # salam resolves std/ from the directory holding its own binary, and a
    # $PATH lookup gives it no directory to work from, so the two are
    # installed together and the wrapper pins SALAM_STD.
    libexec.install "salam"
    libexec.install "std"
    bin.write_env_script libexec / "salam", SALAM_STD: libexec / "std"

    doc.install "README.md"
  end

  test do
    (testpath / "hello.salam").write <<~SALAM
      func main:
          println "salam"
      end
    SALAM

    assert_equal "salam", shell_output("#{bin}/salam exec #{testpath}/hello.salam").strip
    assert_match version.to_s, shell_output("#{bin}/salam version")
  end
end
