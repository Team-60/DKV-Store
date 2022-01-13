[![CMake](https://github.com/Team-60/DKV-Store/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/Team-60/DKV-Store/actions/workflows/cmake.yml)

# DKV-Store
**An on-disk, high-performant & consistent distributed key value store in C++**

## Components
  - Shard master: Redirect a request to Volume server
  - Volume server: Database storing key-value pairs

## Tools & Libraries used
  - [gRPC](https://grpc.io/)
  - [levelDB](https://github.com/google/leveldb)
  - [concurrent-queue](https://github.com/cameron314/concurrentqueue)
  - [thread-pool](https://github.com/bshoshany/thread-pool)
  - [md5 hashing](http://www.zedwood.com/article/cpp-md5-function)

## Build and Run
**Dependencies**:
  - gRPC: [Install](https://grpc.io/docs/languages/cpp/quickstart/#install-grpc)
  - levelDB: [Install](https://github.com/google/leveldb)
  - cmake: [Install](https://cmake.org/install/)

The project uses `cmake > 3.5.1` to generate build files. After installing dependencies, run `bash build.sh` to generate build files. 

### To run tests:
  - Navigate to `tests`
  - Run `bash test.sh`

### To manually spawn servers & clients:
*Write your own instructions using APIs provided in `client/client.cc`*
  - Navigate to `client`
  - To launch shard-master & volume-servers, run `bash launch.sh $num-of-volume-servers`
  - Finally to launch your client, run `../cmake/build/client`


## Dev Workflows
Tests are based of [here](https://cs.brown.edu/courses/csci1310/2020/assign/projects/project5.html) and are automated on Github Actions.
 
## Code Structure
![image](https://user-images.githubusercontent.com/55681240/149201365-b64fa176-891e-4511-bbe4-eff1555dda5a.png)

## License 
MIT
