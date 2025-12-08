{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    threadpool.url = "github:negrel/threadpool.h";
  };

  outputs =
    {
      nixpkgs,
      flake-utils,
      threadpool,
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
              buildInputs = with pkgs; [ clang-tools ];

              THREADPOOL_INCLUDE = "${threadpool.packages.${system}.default}/include";
            };
          };
        }
      );
    in
    outputsWithSystem // outputsWithoutSystem;
}
