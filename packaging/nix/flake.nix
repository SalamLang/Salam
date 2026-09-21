{
  description = "The Salam programming language";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachSystem
      [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ]
      (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          salam = pkgs.callPackage ./package.nix { };
        in
        {
          packages = {
            inherit salam;
            default = salam;
          };

          apps.default = {
            type = "app";
            program = "${salam}/bin/salam";
          };

          devShells.default = pkgs.mkShell {
            packages = [ salam ];
          };
        }
      );
}
