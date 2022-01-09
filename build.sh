## flag "D" provided -> rebuild
if [[ $* == *D* ]]; then echo "* Rebuilding..." && rm -rf cmake; fi
## flag "F" provided -> auto format
if [[ $* == *F* ]]; then 
	echo "* Formatting Project..."
	find . \( -name "*.h" -or -name "*.cc" \) ! -path "./cmake/*" ! -path "./utils/concurrentqueue/*" \
		-exec clang-format -i -style=file {} \; \
		-exec echo "- formatting " {} \;
fi
## flag "T" provided -> build tests
BUILD_TESTS="OFF"
if [[ $* == *T* ]]; then 
	echo "* Build tests detected..."
	BUILD_TESTS="ON"
fi
echo "-------------"

# Create build directory
mkdir -p cmake/build
pushd cmake/build

# build
cmake -DCMAKE_PREFIX_PATH=$HOME/src/builds/gRPC -DLEVELDB_PREFIX=$HOME/src/builds/leveldb -DBUILD_TESTS=$BUILD_TESTS ../..
make

popd
