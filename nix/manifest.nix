{ pkgs ? import <nixpkgs> {}, system ? builtins.currentSystem }:

# Exploring the manifest.json
#
# Find versions that are relevant to a system:
#
# jq -S '."toolchain-atmelavr"[] | {version: .version,system: .system} | select(.system | index("linux_x86_64"))' nix/manifest.jso
#

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
                    then (e.system == "*" || e.system == manifestSystem)
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

  findVersion' = vprefix: entries:
    let
      default = { version = ""; };
    in
      findFirst (p: hasPrefix vprefix p.meta.manifest.version)
        default entries;
in
rec {
  inherit manifest;
  
  findVersion = vprefix: pick:
    let pkg = (pick all);
    in (findVersion' vprefix pkg);
  
  all = mapAttrs (name: entries:
    map (entry: pkgs.fetchurl {
      inherit (entry) url sha1;
      meta = {
        manifest = {
          inherit name entry;
          inherit (entry) version;
        };
      };
    }) entries) manifest;
  
  latest = mapAttrs (name: entries: (last entries)) all;
}
