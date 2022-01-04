#!/bin/bash
: '
Args:
1) Number of volume_servers to spawn
2) flag "D" for complete rebuild / "F" for formatting (Passed directly to ../build.sh)
'

## constants
BASE_ADDR=127.0.0.1

# check for args
if [ $# -lt 1 ]; then 
    echo "Missing args" >&2
    exit 2
fi

BUILD_ARGS=""
if [ $# -eq 2 ]; then 
    BUILD_ARGS=$2
fi

# build
pushd ..
echo

echo "--------------- STARTING BUILD ---------------"
bash build.sh $BUILD_ARGS
echo "--------------- BUILD COMPLETE ---------------"
echo 

popd
echo

# run
pushd ../cmake/build
echo

## run shard-master
echo "> running shard_master at $BASE_ADDR:8080"
./shard_master & shard_master_pid=$!

sleep 2

## run initial volume servers
for ((i = 1; i <= $1; i ++))
do 
    let PORT=8080+i
    echo "> running volume_server $i at $BASE_ADDR:$PORT"
    ./volume_server $BASE_ADDR:$PORT $i & pid=$!
    volume_server_pids+=" $pid"
    sleep 0.5
done

trap "kill $shard_master_pid $volume_server_pids" SIGINT

wait $shard_master_pid $volume_server_pids

popd
