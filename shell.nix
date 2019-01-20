{ pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  name = "keyboard-flash";
  buildInputs = with pkgs; [ platformio ];
}
