{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    threadpool.url = "github:negrel/threadpool.h";
    mpsc.url = "github:negrel/mpsc.h";
  };

  outputs =
    {
      nixpkgs,
      flake-utils,
      threadpool,
      mpsc,
      ...
    }:
    let
      outputsWithoutSystem = { };
      outputsWithSystem = flake-utils.lib.eachDefaultSystem (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in
        {
          devShells = {
            default = pkgs.mkShell {
              buildInputs = with pkgs; [
                clang-tools
                valgrind
              ];

              THREADPOOL_INCLUDE = "${threadpool.packages.${system}.default}/include";
              MPSC_INCLUDE = "${mpsc.packages.${system}.default}/include";
            };
          };
        }
      );
    in
    outputsWithSystem // outputsWithoutSystem;
}
