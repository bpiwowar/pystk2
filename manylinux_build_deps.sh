# Builds missing dependencies for manylinux

mkdir deps

curl --output deps/sdl2.tar.gz -L https://github.com/libsdl-org/SDL/releases/download/release-2.28.4/SDL2-2.28.4.tar.gz
# sha5sum -c -a 256 <(echo deps/sdl2.tar.gz 888b8c39f36ae2035d023d1b14ab0191eb1d26403c3cf4d4d5ede30e66a4942c)
tar vxf -C deps/sdl2 deps/sdl2.tar.gz
(cd SDL2-2.28.4 && ./configure && make install)