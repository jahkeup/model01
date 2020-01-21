{ pkgs ? import <nixpkgs> {}, system ? builtins.currentSystem }:

with pkgs.lib;
with builtins;

let
  raw = importJSON ./manifest.json;
  manifestSystem = {
    "x86_64-linux"  = "linux_x86_64";
    "aarch64-linux" = "linux_aarch64";
  }."${system}";
  matchingSystems = [ manifestSystem ];
  isEntryMatch = e: if (isString e.system)
                    then (e.system == "*")
                    else (length (intersectLists e.system matchingSystems)) > 0;
  filterManifestItem = itemName: itemEntries:
    let
      scoped = filter isEntryMatch itemEntries;
      entryVersion = a: b: (head (naturalSort [ a.version b.version ])) == a.version;
    in sort entryVersion scoped;
  filterManifest = items:
    let
      scoped = mapAttrs filterManifestItem items;
    in
      filterAttrs (_: itemEntries: (length itemEntries) > 0) scoped;

  manifest = filterManifest raw;
in
rec {
  inherit manifest;
  all = mapAttrs (name: entries:
    map (entry: pkgs.fetchurl {
      inherit (entry) url sha1;
      meta = {
        manifest = { inherit name entry; };
      };
    }) entries) manifest;
  latest = mapAttrs (name: entries: (last entries)) all;
}
