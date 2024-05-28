{
  stdenv,
  cmake,
  pkg-config,
  installShellFiles,
  libclang,
  clang,
  llvmPackages,
  libllvm,
  yaml-cpp,
  enableLibcxx ? false,
}:
stdenv.mkDerivation {
  name = "clang-uml";
  src = ../..;

  # variables for substituteAll
  unwrapped = llvmPackages.clang-unwrapped;
  clang = if enableLibcxx then llvmPackages.libcxxClang else llvmPackages.clang;

  nativeBuildInputs = [
    cmake
    pkg-config
    installShellFiles
  ];

  buildInputs = [
    clang
    libclang
    libllvm
    yaml-cpp
  ];

  postInstall = ''
    substituteAll ${./wrapper} $out/bin/clang-uml-wrapped
    installShellCompletion --bash $src/packaging/autocomplete/clang-uml
    installShellCompletion --zsh $src/packaging/autocomplete/_clang-uml
  '';
}
