{
  description = "ezpwd-reed-solomon - Reed-Solomon & BCH encoding and decoding, in C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        commonInputs = with pkgs; [
          # Common tools
          cacert
          git
          gnumake
          gnused
          bash
          which
          jq
          curl

          # C++ toolchain from nixpkgs.  This also supplies a working C
          # compiler/linker via the stdenv, so builds do not depend on the
          # host having Xcode or build-essential installed.
          gcc
          cmake
          ninja

          # Test-suite dependencies (see GNUmakefile)
          perl
          ncurses
          boost
        ];
      in {
        # The project is distributed as header-only; local builds exist
        # solely to run the test-suite and examples.  No package derivation.
        devShells.default = pkgs.mkShell {
          packages = commonInputs;
          shellHook = ''
            echo "ezpwd-reed-solomon - Reed-Solomon & BCH codecs (header-only) development environment"
            echo ""
            printf "  %-12s %s\n" "g++"    "$(g++ --version 2>/dev/null | head -1)"
            printf "  %-12s %s\n" "cmake"  "$(cmake --version 2>/dev/null | head -1)"
            printf "  %-12s %s\n" "ninja"  "$(ninja --version 2>/dev/null)"
            printf "  %-12s %s\n" "make"   "$(make --version 2>/dev/null | head -1)"
            printf "  %-12s %s\n" "perl"   "$(perl -e 'print $^V')"
            echo ""
            echo "Commands:  make test      # C++ test-suite (GNUmakefile)"
            echo "           make testex    # C++ test executables only"
          '';
        };
      });
}
