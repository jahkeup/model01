{ pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  name = "model01-dev";
  buildInputs = with pkgs; [
    platformio gnumake
    ccls clang-tools
  ];
}
