{ pkgs ? import ./nixpkgs.nix {}, shell ? false }:

with pkgs;

mkShell.override { stdenv = pkgs.gcc11Stdenv; } {
  inputsFrom = lib.attrValues  (import ./. {
    inherit pkgs;
    shell = true;
  });

  buildInputs = [
    # additional packages go here
    ncurses
  ];
}
