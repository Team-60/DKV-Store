# Create build directory
mkdir -p cmake/build

pushd cmake/build

cmake -DCMAKE_INSTALL_PREFIX=$HOME/src/builds/gRPC -DLEVELDB_PREFIX=$HOME/src/builds/leveldb ../..

make

popd
