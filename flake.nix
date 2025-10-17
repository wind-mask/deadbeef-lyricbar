
{
  description = "A deadbeef plugin for displaying lyrics";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        gtk3 = pkgs.gtk3;
        gtkmm3 = pkgs.gtkmm3;
        taglib = pkgs.taglib;
        curl = pkgs.curl;
        glib = pkgs.glib;
        gettext = pkgs.gettext;
        deadbeef = pkgs.deadbeef;
      in
      {
        packages.default = pkgs.stdenv.mkDerivation rec {
          pname = "deadbeef-lyricbar";
          version = "git";

          src = self;

          nativeBuildInputs = [
            pkgs.pkg-config
            pkgs.gettext
          ];

          buildInputs = [
            gtk3
            gtkmm3
            taglib
            curl
            glib
            deadbeef
          ];

          buildPhase = ''
            runHook preBuild
            
            glib-compile-resources --generate-header --target=src/resources.h src/resources.xml
            glib-compile-resources --generate-source --target=src/resources.c src/resources.xml

            ${pkgs.gnumake}/bin/make gtk3
            
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall

            install -d $out/lib/deadbeef
            install -m 644 ddb_lyricbar_gtk3.so $out/lib/deadbeef/
            install -d $out/share/locale/ru/LC_MESSAGES
            msgfmt gettext/ru/deadbeef-lyricbar.po -o $out/share/locale/ru/LC_MESSAGES/deadbeef-lyricbar.mo

            runHook postInstall
          '';
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];
        };
      });
}
