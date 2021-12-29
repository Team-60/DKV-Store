# Create build directory
rm -rf cmake
mkdir -p cmake/build

pushd cmake/build

# Current Installation Path
export MY_INSTALL_DIR=$HOME/.local

cmake -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR ../..
make

popd
