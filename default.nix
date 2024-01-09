{ pkgs }:
pkgs.stdenv.mkDerivation rec {
  pname = "dpp";
  version = "10.0.29";

  src = pkgs.fetchFromGitHub {
    owner = "brainboxdotcc";
    repo = "DPP";
    rev = "v${version}";
    sha256 = "BJMg3MLSfb9x/2lPHITeI3SWwW1OZVUUMVltTWUcw9I=";
  };

  nativeBuildInputs = with pkgs; [ cmake autoPatchelfHook libopus];
  buildInputs = with pkgs; [ openssl zlib libsodium gcc ];

  meta = with pkgs.lib; {
    description = "D++ Discord C++ Library";
    homepage = "https://github.com/brainboxdotcc/DPP";
    license = pkgs.lib.licenses.asl20;
    maintainers = with pkgs.lib.maintainers; [ renato ];
  };
}
