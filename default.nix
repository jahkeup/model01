{ stdenv, platformio }:
stdenv.mkDerivation {
  name = "keyboardio-firmware";
  # TODO: Make the library sources managed by nix with generated
  # platformio.ini.
  src = ./.;
  buildInputs = [ platformio ];
  buildPhase = ''
    pio run
  '';
  installPhase = ''
    install -D .pio/build/model01/firmware.hex $out/firmware.hex
  '';
}
