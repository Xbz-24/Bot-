{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [ 
	  (pkgs.callPackage ~/Nix-Stuff/DPP/default.nix {
		openssl = openssl.out;
		zlib = zlib.out;
		libsodium = libsodium.out;
		opus = libopus.out;
		gcc = gcc-unwrapped.lib;
	  })
	gcc
	cmake
  ];
}
