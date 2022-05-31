{
  description = "Uinput helper library for C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    strutcpp.url = "github:grumbel/strutcpp";
    strutcpp.inputs.nixpkgs.follows = "nixpkgs";
    strutcpp.inputs.flake-utils.follows = "flake-utils";
    strutcpp.inputs.tinycmmc.follows = "tinycmmc";

    logmich.url = "github:logmich/logmich";
    logmich.inputs.nixpkgs.follows = "nixpkgs";
    logmich.inputs.flake-utils.follows = "flake-utils";
    logmich.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc, strutcpp, logmich }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          uinpp = pkgs.stdenv.mkDerivation {
            pname = "uinpp";
            version = "0.1.0";
            src = nixpkgs.lib.cleanSource ./.;
            cmakeFlags = [ "-DBUILD_TESTS=ON" ];
            doCheck = true;
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkgconfig
            ];
            buildInputs = [
              tinycmmc.defaultPackage.${system}
              logmich.defaultPackage.${system}
              strutcpp.defaultPackage.${system}

              pkgs.util-linux
              pkgs.gtest
              pkgs.fmt
            ];
           };
        };
        defaultPackage = packages.uinpp;
      });
}
