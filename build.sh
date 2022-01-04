# Create build directory
## flag "D" provided -> rebuild
if [[ $* == *D* ]]; then echo "* Rebuilding..." && rm -rf cmake; fi
## flag "F" provided -> auto format
EXCLUDE_FOLDER=./cmake
if [[ $* == *F* ]]; then 
	echo "* Formatting Project..."
	find . \( -name "*.h" -or -name "*.cc" \) -not -path "${EXCLUDE_FOLDER}" \
		-exec clang-format -i -style=file {} \; \
		-exec echo "- formatting " {} \;
fi
echo "-------------"

mkdir -p cmake/build

pushd cmake/build

cmake -DCMAKE_PREFIX_PATH=$HOME/src/builds/gRPC -DLEVELDB_PREFIX=$HOME/src/builds/leveldb ../..

make

popd
