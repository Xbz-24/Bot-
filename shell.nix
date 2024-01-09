{ pkgs ? import <nixpkgs> {} }:
let
   dpp = import ./default.nix { inherit pkgs; };
in
pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    dpp
	openssl
	zlib
	libsodium
	libopus
	pkg-config
	ffmpeg_5
	youtube-dl
  ];
  shellHook = with pkgs; ''
    echo ""
    echo "----------------------------------------shellHook--------------------------------------"
    echo "ffmpeg path               : ${ffmpeg_5}"
    echo "libopus path              : ${libopus}"
    echo "libsodium path            : ${libsodium}"
    echo "zlib path                 : ${zlib}"
    echo "openssl path              : ${openssl}"
    echo "dpp path                  : ${dpp}"
    echo "cmake path                : ${cmake}"
    echo "youtube-dl path           : ${cmake}"
    echo "---------------------------------------------------------------------------------------"
  '';
}
