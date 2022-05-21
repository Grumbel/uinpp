{
  description = "Uinput helper library for C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "gitlab:grumbel/cmake-modules";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    strutcpp.url = "gitlab:grumbel/strutcpp";
    strutcpp.inputs.nixpkgs.follows = "nixpkgs";
    strutcpp.inputs.flake-utils.follows = "flake-utils";
    strutcpp.inputs.tinycmmc.follows = "tinycmmc";

    logmich.url = "gitlab:logmich/logmich";
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
          uinputcpp = pkgs.stdenv.mkDerivation {
            pname = "uinputcpp";
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
        defaultPackage = packages.uinputcpp;
      });
}
