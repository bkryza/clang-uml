{
  description = "C++ UML diagram generator based on Clang";

  inputs = {
    nixpkgs.url = "nixpkgs";

    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
  };

  outputs = {flake-parts, ...} @ inputs:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];

      perSystem = {
        self',
        pkgs,
        ...
      }: {
        packages = {
          default = self'.packages.clang-uml;
          clang-uml = pkgs.callPackage ./packaging/nix {};
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [self'.packages.clang-uml];
        };

        formatter = pkgs.alejandra;
      };
    };
}
