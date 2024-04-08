{
  description = "Description for the project";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [];
      systems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin" ];
      perSystem = { config, self', inputs', pkgs, system, ... }: {

        packages.default = config.packages.clang-uml;
        packages.clang-uml = pkgs.callPackage packaging/nix/default.nix { };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # C++ Compiler is already part of stdenv
            cmake
            llvmPackages_latest.libllvm
            yaml-cpp
            ccache
            elfutils
            pkg-config
            clang
            libclang
          ];
        };
      };
    };
}
