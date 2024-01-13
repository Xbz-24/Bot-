{ pkgs ? import <nixpkgs> {} }:
let
   dpp = import ./default.nix { inherit pkgs; };
   unstable = import (builtins.fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz") {};
in
pkgs.mkShell {
  buildInputs = with pkgs; [
    unstable.fmt
    cmake
    dpp
    openssl
    zlib
    libsodium
    libopus
    opusfile.dev
    pkg-config
    ffmpeg_5
    youtube-dl
    yt-dlp
    unstable.boost183.dev
    unstable.liboggz
    unstable.ninja
  ];
  #     export BOOST_ROOT=${unstable.boost183.dev}
  shellHook = with pkgs; ''
    echo ""
    echo "ffmpeg path               : ${ffmpeg_5}"
    echo "libopus path              : ${libopus}"
    echo "libsodium path            : ${libsodium}"
    echo "zlib path                 : ${zlib}"
    echo "openssl path              : ${openssl}"
    echo "dpp path                  : ${dpp}"
    echo "cmake path                : ${cmake}"
    echo "youtube-dl path           : ${youtube-dl}"
    echo "yt-dlp path               : ${yt-dlp}"
    echo "boost path                : ${unstable.boost183.dev}"
  '';
}
