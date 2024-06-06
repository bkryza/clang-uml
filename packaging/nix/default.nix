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

  clang = if enableLibcxx then llvmPackages.libcxxClang else llvmPackages.clang;

  postInstall = ''
    cp $out/bin/clang-uml $out/bin/clang-uml-unwrapped
    rm $out/bin/clang-uml
    export unwrapped_clang_uml="$out/bin/clang-uml-unwrapped"

    # inject clang and unwrapp_clang_uml variables into wrapper
    substituteAll ${./wrapper} $out/bin/clang-uml
    chmod +x $out/bin/clang-uml

    installShellCompletion --cmd clang-uml \
      --bash $src/packaging/autocomplete/clang-uml \
      --zsh $src/packaging/autocomplete/_clang-uml
  '';
}
