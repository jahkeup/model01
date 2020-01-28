build: headers
	nix-shell shell.nix --run 'pio run'

ci:
	nix-build --option sandbox false ci.nix

.pio/libdeps/keyboardio/KaleidoscopeScanner/KeyboardioScanner.h: platformio.ini
	nix-shell shell.nix --run 'pio lib install'

headers: .pio/libdeps/keyboardio/KaleidoscopeScanner/KeyboardioScanner.h

