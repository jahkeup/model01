{ pkgs, lib, stdenv, runCommand, mkShell, platformio, lndir, fetchFromGitHub }:

with lib;

let
  pioPackages = import ./manifest.nix { inherit pkgs; };

  pioEnv = let
    packages = let
      renamed = name: p: p //
                         { meta = p.meta // { overrideName = name; }; };
    in {
      platforms = with pioPackages.latest; [
        #(renamed toolchain-atmelavr "atmelavr")

        (renamed "atmelavr" (fetchFromGitHub {
          owner = "platformio";
          repo = "platform-atmelavr";
          rev = "de57dda3eb6193d19ab51136d3347b8e56933a00";
          sha256 = "1x6qmhi5m4lczan20yx5xgyy2rdip9bhcka2jiw0f62qwx2g8j1w";
        }))
      ]
      ;
      packages = with pioPackages.latest; [
        framework-arduinoavr
        framework-arduino-avr
        toolchain-atmelavr
        tool-scons
      ];
    };

    linkPackage = pkg: let
      name = pkg.meta.overrideName or pkg.meta.manifest.name;
      pioPackageDir = runCommand name { src = pkg; }
        ''mkdir $out; cd $out; unpackFile $src'';
    in ''
    echo "Adding ${name} (${pioPackageDir})";
    ln -snfv ${pioPackageDir} ${name};
  '';
    linkPackages = concatMapStrings linkPackage;
  in
    runCommand "platformio-libs"
      { passthru = { platformio = packages; }; }
      ''
        mkdir -p $out/platformio/lib \
                 $out/platformio/packages \
                 $out/platformio/platforms
        cd $out/platformio/platforms
        echo "Linking platform into environment"
        ${linkPackages packages.platforms}
        cd $out/platformio/packages
        echo "Linking packages into environment"
        ${linkPackages packages.packages}
        ${linkPackages packages.platforms}
      '';

  shell = mkShell {
    PLATFORMIO_LIB_EXTRA_DIRS = "${pioEnv}/platformio/lib";
    buildInputs = [ drv platformio ];
  };

  drv = stdenv.mkDerivation {
    name = "model01";
    src = ./..;

    PLATFORMIO_CORE_DIR = ".nix-pio";
    PLATFORMIO_LIB_EXTRA_DIRS = "${pioEnv}/platformio/lib";
    PLATFORMIO_SETTING_ENABLE_TELEMETRY = false;

    buildInputs = [ platformio lndir ];
    postConfigure = ''
      mkdir -p $PLATFORMIO_CORE_DIR/{platforms,lib,packages,cache}
    '';
    buildPhase = let
      p = pkg: with pkg; ''${meta.overrideName or meta.manifest.name}=${out}'';
    in ''
      platformio platform install ${p (last pioEnv.platformio.platforms)} --with-package ${p (last pioEnv.platformio.packages)}
    '';
    installPhase = ''
      ls -al .pio
    '';
  };

in
{
  firmware = drv;
  environment = pioEnv;
  shell = shell;
}
