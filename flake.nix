{
  description = "A deadbeef plugin for displaying lyrics";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        # 定义一个函数来构建包，接受 debug 参数
        mkPackage =
          debug:
          pkgs.stdenv.mkDerivation {
            pname = "deadbeef-lyricbar";
            version = "git";

            src = self;

            nativeBuildInputs = [
              pkgs.pkg-config
              pkgs.gettext
            ];

            buildInputs = [
              pkgs.gtk3
              pkgs.gtkmm3
              pkgs.taglib
              pkgs.curl
              pkgs.glib
              pkgs.deadbeef
            ];

            buildPhase = ''
              runHook preBuild

              glib-compile-resources --generate-header --target=src/resources.h src/resources.xml
              glib-compile-resources --generate-source --target=src/resources.c src/resources.xml

              ${pkgs.gnumake}/bin/make -j$(nproc) ${if debug then "debug" else "gtk3"}

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
      in
      {
        # 默认包（release 模式）
        packages.default = mkPackage false;

        # Debug 包
        packages.debug = mkPackage true;

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];
        };
      }
    );
}
