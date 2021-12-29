# Create build directory
## flag "D" provided -> rebuild
if [ $# -eq 1 ]; then
	if [ $1 == "D" ]; then echo "Rebuilding..." && rm -rf cmake; fi
fi
mkdir -p cmake/build

pushd cmake/build

cmake -DCMAKE_INSTALL_PREFIX=$HOME/src/builds/gRPC -DLEVELDB_PREFIX=$HOME/src/builds/leveldb ../..

make

popd
