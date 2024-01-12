{ pkgs }:
let
   libExt = if pkgs.stdenv.hostPlatform.isDarwin then ".dylib" else ".so";
in
pkgs.stdenv.mkDerivation rec {
  pname = "dpp";
  version = "10.0.29";

  src = pkgs.fetchFromGitHub {
    owner = "brainboxdotcc";
    repo = "DPP";
    rev = "v${version}";
    sha256 = "BJMg3MLSfb9x/2lPHITeI3SWwW1OZVUUMVltTWUcw9I=";
  };

  nativeBuildInputs = with pkgs; [ 
    cmake 
    libopus
  ] ++ lib.optional (stdenv.hostPlatform.isLinux) autoPatchelfHook;

  buildInputs = with pkgs; [ 
    openssl 
    zlib 
    libsodium 
    libopus
    gcc 
    pkg-config
  ];

  PKG_CONFIG_PATH = with pkgs; [
    "${openssl.dev}/lib/pkgconfig"
    "${libsodium.dev}/lib/pkgconfig"
    "${libopus.dev}/lib/pkgconfig"
  ];

  cmakeFlags = with pkgs; [
    "-DLibsodium_INCLUDE_DIR=${pkgs.libsodium.dev}/include"
    "-DLibsodium_LIBRARY=${pkgs.libsodium.out}/lib"
    "-DOPENSSL_ROOT_DIR=${openssl.dev}"
    "-DOPENSSL_LIBRARIES=${openssl.out}/lib"
    "-DOPENSSL_CRYPTO_LIBRARY=${openssl.out}/lib/libcrypto${libExt}"
    "-DOPENSSL_SSL_LIBRARY=${openssl.out}/lib/libssl${libExt}"
    "-DOPENSSL_INCLUDE_DIR=${openssl.dev}/include"
  ];

  meta = with pkgs.lib; {
    description = "D++ Discord C++ Library";
    homepage = "https://github.com/brainboxdotcc/DPP";
    license = pkgs.lib.licenses.asl20;
    maintainers = with pkgs.lib.maintainers; [ renato ];
  };
}
