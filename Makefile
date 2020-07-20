PIO_ENV=model01

# Issues have been had with Platformio versions:
#
# - v4.1.0 :: dropped 'includes'

build: .pio/build/$(PIO_ENV)/firmware.hex

firmware: build-out/firmware.hex

check: cpplint

release: build check

ci: release build-out/firmware.hex

cpplint:
	cpplint --extensions=ino,cpp,c,h,hpp $(wildcard src/*.*)

.pio/build/$(PIO_ENV)/firmware.hex: $(wildcard src/*) $(wildcard include/) platformio.ini
	pio run --environment $(PIO_ENV)

build-out/firmware.hex: build
	rm -rf ./build-out
	mkdir build-out
	cp .pio/build/$(PIO_ENV)/firmware.hex ./build-out/firmware.hex

.ccls .clang_complete:
	pio init --ide=emacs

ugh-pio:
	pio run --environment $(PIO_ENV) --target envdump

clean:
	rm -rf .pio/
	rm -rf .ccls .ccls-cache/
	rm -rf build-out
