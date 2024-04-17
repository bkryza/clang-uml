{
  stdenv,
  cmake,
  pkg-config,
  installShellFiles,
  libclang,
  libllvm,
  yaml-cpp,
}:
stdenv.mkDerivation {
  name = "clang-uml";
  src = ../..;

  nativeBuildInputs = [
    cmake
    pkg-config
    installShellFiles
  ];

  buildInputs = [
    libclang
    libllvm
    yaml-cpp
  ];

  postInstall = ''
    installShellCompletion --bash $src/packaging/autocomplete/clang-uml
    installShellCompletion --zsh $src/packaging/autocomplete/_clang-uml
  '';
}
