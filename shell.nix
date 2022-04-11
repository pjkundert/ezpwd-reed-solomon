{ pkgs ? import ./nixpkgs.nix {}, shell ? false }:

with pkgs;

mkShell {
  inputsFrom = lib.attrValues  (import ./. {
    inherit pkgs;
    shell = true;
  });

  buildInputs = [
    # additional packages go here
  ];
}
