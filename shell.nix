{ compiler ? "",
  nixpkgs ? (import <nixpkgs> {}).fetchFromGitHub {
    owner  = "NixOS";
    repo   = "nixpkgs";
    rev    = "053ad4e0db7241ae6a02394d62750fdc5d64aa9f";
    sha256 = "11l9sr8zg8j1n5p43zjkqwpj59gn8c84z1kf16icnsbnv2smzqdc";
  }}:

with import nixpkgs {};

let
  # For the documentation tools we use an older Nixpkgs since the
  # newer versions seem to be not working great...
  old-nixpkgs-src = fetchFromGitHub {
                      owner  = "NixOS";
                      repo   = "nixpkgs";
                      rev    = "d0d905668c010b65795b57afdf7f0360aac6245b";
                      sha256 = "1kqxfmsik1s1jsmim20n5l4kq6wq8743h5h17igfxxbbwwqry88l";
                    };
  old-nixpkgs     = import old-nixpkgs-src {};
  docs            = import ./nix/docs.nix { nixpkgs = old-nixpkgs-src; };
  deps            = import ./nix/deps.nix { inherit nixpkgs; };
  compilerPkg   = if compiler != ""
                  then pkgs.${compiler}
                  else stdenv.cc;
  theStdenv     = if compilerPkg.isClang
                  then clangStdenv
                  else stdenv;
  qt            = qt5;
  qtver         = qt.qtbase.version;

in
theStdenv.mkDerivation rec {
  name = "lager-env";
  buildInputs = [
    cmake
    ccache
    ncurses
    boost
    pkgconfig
    libiconvReal
    glibcLocales
    deps.libhttpserver
    deps.cereal
    deps.immer
    deps.zug
    deps.imgui
    sass
    SDL2
    SDL2_ttf
    emscripten
    qt.qtbase
    qt.qtdeclarative
    qt.qtquickcontrols
    qt.qtquickcontrols2
    qt.qtgraphicaleffects
    (python3.withPackages (pkgs: with pkgs; [
      click
      requests
    ]))
    old-nixpkgs.doxygen
    (old-nixpkgs.python.withPackages (ps: [
      ps.sphinx
      docs.breathe
      docs.recommonmark
      ]))
  ] ++ lib.optionals stdenv.isLinux [
    old-nixpkgs.elmPackages.elm-reactor
    old-nixpkgs.elmPackages.elm-make
    old-nixpkgs.elmPackages.elm-package
  ];
  shellHook = ''
    export LAGER_ROOT=`dirname ${toString ./shell.nix}`
    export LAGER_RESOURCES_PATH="$LAGER_ROOT"/resources
    addToSearchPath PATH "$LAGER_ROOT/build"
    addToSearchPath PATH "$LAGER_ROOT/build/example"
    addToSearchPath PATH "$LAGER_ROOT/build/test"
    addToSearchPath QML2_IMPORT_PATH ${qt512.qtquickcontrols}/lib/qt-${qtver}/qml
    addToSearchPath QML2_IMPORT_PATH ${qt512.qtquickcontrols2.bin}/lib/qt-${qtver}/qml
    addToSearchPath QML2_IMPORT_PATH ${qt512.qtgraphicaleffects}/lib/qt-${qtver}/qml
    addToSearchPath QT_PLUGIN_PATH ${qt512.qtsvg.bin}/lib/qt-${qtver}/plugins
    export QT_QPA_PLATFORM_PLUGIN_PATH=${qt512.qtbase}/lib/qt-${qtver}/plugins
    export IMGUI_SOURCE_DIR=${deps.imgui}
  '';
}
