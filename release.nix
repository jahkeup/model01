{ nixpkgs ? import <nixpkgs> {} }:
with nixpkgs;
{
  firmware = (callPackage ./default.nix {});
}
