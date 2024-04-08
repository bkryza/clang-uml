{ pkgs ? import <nixpkgs> {} }:


# Nix derivation for basic C++ project using clang
with pkgs; stdenv.mkDerivation {
  name = "clang-uml";
  src = ../..;

  buildInputs = [
    clang
    libclang
    cmake
    llvmPackages_latest.libllvm
    yaml-cpp
    ccache
    elfutils
    pkg-config
  ];

  dontUseCmakeConfigure = true;

  buildPhase = "CCACHE_DIR=/build/.ccache make release";

  installPhase = ''
    mkdir -p $out/bin
    cp release/src/clang-uml $out/bin/clang-uml
  '';

  postInstall = "
    installShellCompletion --cmd clang-uml \
      --zsh packaging/autocomplete/_clang-uml
      --bach packaging/autocomplete/clang-uml
  ";
}
