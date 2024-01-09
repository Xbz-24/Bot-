{ 
   stdenv 
   ,lib 
   ,fetchurl 
   ,autoPatchelfHook 
   ,openssl
   ,zlib
   ,libsodium
   ,opus
   ,gcc
}:

stdenv.mkDerivation rec {
  pname = "dpp";
  version = "10.0.29";

  src = fetchurl {
    url = "https://github.com/brainboxdotcc/DPP/releases/download/v${version}/libdpp-${version}-linux-x64.deb";
    sha256 = "11ssfmba0cdipls2f2fgsb8dp6fbx3mkk6y18kzgzcdhsvh9mgk8";
  };

  nativeBuildInputs = [ autoPatchelfHook ];

  unpackPhase = ''
    ar x $src
    tar xvf data.tar.gz
  '';

  installPhase = ''
    mkdir -p $out
    ls -l ${libsodium}/lib
    cp -R usr/* $out/
  '';


  postFixup = ''
    patchelf --set-rpath $ORIGIN/../${lib.makeLibraryPath [ openssl zlib libsodium opus gcc ]} $out/lib/libdpp.so.10.0.29
    patchelf --replace-needed libsodium.so.23 libsodium.so.26 $out/lib/libdpp.so.10.0.29
  '';

  meta = with lib; {
    description = "D++ Discord C++ Library";
    homepage = "https://github.com/brainboxdotcc/DPP";
    license = licenses.asl20; 
  };
}
