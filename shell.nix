let
  pkgs = import <nixpkgs> {
    system = "aarch64-darwin";
  };
in
  pkgs.mkShell {
    packages = [
      pkgs.stdenv
    ];
  }
