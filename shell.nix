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
  ];
}
