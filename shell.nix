{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "FreeJam";
  buildInputs = with pkgs; [
    pkg-config
    alsa-lib.dev
    python39
    python39Packages.cffi
  ];

  shellHook = ''
  # Bash goes here
  export FOO=BAR
  echo "Starting new shell"
  '';
}
