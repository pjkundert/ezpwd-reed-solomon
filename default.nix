{ pkgs ? import ./nixpkgs.nix {}, shell ? false }:

with pkgs;

let
  inherit (rust.packages.stable) rustPlatform;
  inherit (darwin.apple_sdk.frameworks) Security;
in

{
  ezpwd-reed-solomon = stdenv.mkDerivation rec {
    name = "ezpwd-reed-solomon";
    src = gitignoreSource ./.;

    cargoSha256 = "0xz29dxvg306kfc6fmysccdlw8fldwdpv9i40dwxnmd0wzcwhxxx";

    nativeBuildInputs = [ pkg-config ] ++ lib.optionals stdenv.isDarwin [
      xcbuild
    ];

    buildInputs = [
      git
      perl
      bash
      boost
    ];

    buildPhase = ''
      make
    '';

    installPhase = ''
      mkdir -p          $out/include
      cp -RL c++/*      $out/include
      mkdir -p          $out/lib
      mv libezpwd-*     $out/lib
    '';
  };
}
