{ pkgs, lib, stdenv, runCommand, mkShell, platformio, lndir, rsync, fetchFromGitHub, jq }:

with lib;

let
  pioPackages = import ./manifest.nix { inherit pkgs; };
  platform = fetchFromGitHub {
    name = "platform-atmelvar";
    owner = "platformio";
    repo = "platform-atmelavr";
    rev = "de57dda3eb6193d19ab51136d3347b8e56933a00";
    sha256 = "1x6qmhi5m4lczan20yx5xgyy2rdip9bhcka2jiw0f62qwx2g8j1w";
  };

  deps = rec {
    HID = fetchFromGitHub {
      name = "keyboardio-Arduino-HID";
      repo = "Arduino-HID";
      owner = "keyboardio";
      rev = "05a9c45c5578ed3b3b74934e7deba7422a6943ab";
      sha256 = "098rzibpnnag8ikimxw0ga79qy4d8d3spg87drw9bcdz4fm6726v";
    };
    KaleidoscopeScanner = let
      src =  fetchFromGitHub {
        name = "keyboardio-KeyboardioScanner";
        repo = "KeyboardioScanner";
        owner = "keyboardio";
        rev = "4224b8843f6eee29fe5e3d05961df2c204940a02";
        sha256 = "0shqa7c1cdqzvxwnpc4hgayz4pg3y97kprwx0f2p9sl7k1q56n28";
      };
    in runCommand "KaleidoscopeScanner" { buildInputs = [ lndir ]; } ''
      mkdir -p $out/
      lndir ${src} $out/

      cat > $out/library.properties <<EOF
      name=KaleidoscopeScanner
      version=0.0.0
      author=Jesse Vincent
      maintainer=Jesse Vincent <jesse@keyboard.io>
      url=https://github.com/keyboardio/KaleidoscopeScanner
      architectures=avr
      dot_a_linkage=false
      EOF
    '';
    KaleidoscopeHID = fetchFromGitHub {
      name = "keyboardio-KeyboardioHID";
      repo = "KeyboardioHID";
      owner = "keyboardio";
      rev = "6cc7ddec763263f936c31546bafcf716e5128f29";
      sha256 = "114sc5pg2a9czhpiq8dixifwg6h6dah1mnj8xl8mfq604ng7khfc";
    };
    Kaleidoscope-HIDAdaptor-KeyboardioHID = fetchFromGitHub {
      name = "keyboardio-HIDAdaptor-KeyboardioHID";
      repo = "Kaleidoscope-HIDAdaptor-KeyboardioHID";
      owner = "keyboardio";
      rev = "e691ec0e323ca671521dfb56a6ff59a0ae7998bb";
      sha256 = "098rzibpnnag8ikimxw0ga79qy4d8d3spg87drw9bcdz4fm6726v";
    };
    Kaleidoscope = fetchFromGitHub {
      name = "keyboardio-Kaleidoscope";
      repo = "Kaleidoscope";
      owner = "keyboardio";
      rev = "116f9b9e71f5ce6779eec1a230f3f5a7e9b6775d";
      sha256 = "0frwqfkywqjvgb4cnqf9rv5vc0b9q632gzk0czk0s1y12rakjrd0";
    };
  };
in

stdenv.mkDerivation {
  name = "pio-try";
  src = ./..;
  
  buildInputs = [ platformio lndir jq rsync ];
  
  configurePhase = ''
    export HOME=$(mktemp -d)
    export PLATFORMIO_PACKAGES_DIR=$HOME/.platformio/packages
    export PLATFORMIO_PLATFORMS_DIR=$HOME/.platformio/platforms
    export PLATFORMIO_WORKSPACE_DIR=$HOME/.pio
    mkdir -p $HOME/.platformio
    (
      cd $HOME/.platformio
      ln -s $src/boards boards
  
      echo '{}' > appstate.json
      
      mkdir -p \
            packages/{tool-scons,framework-arduino-avr,framework-arduinoavr,toolchain-atmelavr} \
            platforms/ \
  
      xargs -t -- tar -C packages/tool-scons -x -f <<< ${pioPackages.findVersion "3.301" (p: p.tool-scons)}
      xargs -t -- tar -C packages/toolchain-atmelavr -x -f <<< ${pioPackages.findVersion "1.5" (p: p.toolchain-atmelavr)}
      xargs -t -- tar -C packages/framework-arduino-avr -x -f <<< ${pioPackages.findVersion "5.0" (p: p.framework-arduino-avr)}
      xargs -t -- tar -C packages/framework-arduinoavr -x -f <<< ${pioPackages.latest.framework-arduinoavr}
  
      mkdir platforms/atmelavr
      lndir ${platform}/ platforms/atmelavr/
  
      # Do tidying operations on the platform.json to convince it the environment
      # is fine.
  
      # rm platforms/atmelavr/platform.json
      # jq 'del(.packages."tool-scons")' ${platform}/platform.json > platforms/atmelavr/platform.json
    )

    ${concatMapStringsSep "\n" (s: "rsync -l --chmod F664,D775 -r ${s.path}/ lib/${s.name}")
      (mapAttrsToList (name: dep: { inherit name; path = dep; }) deps)}
  '';
  
  buildPhase = ''
    pio run --target clean
    pio run
  '';
  installPhase = ''
    install -D .pio/build/keyboardio/firmware.hex $out/firmware.hex
  '';
}
