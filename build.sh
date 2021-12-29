# Create build directory
## flag "D" provided -> rebuild
if [ $# -eq 1 ]; then
	if [ $1 == "D" ]; then echo "Rebuilding..." && rm -rf cmake; fi
fi
mkdir -p cmake/build

pushd cmake/build

# Current Installation Path
export MY_INSTALL_DIR=$HOME/.local

cmake -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR ../..
make

popd
