# BasicWorker

A daemon that consumes assignments from the message queue, builds and runs them 
and then sends results back.

## How to run it

- Install ZeroMQ, version at least 4.0. Package name should be `libzmq3-dev`
- Install dependencies with `git submodule update --init`
- Install CMake, GNU Make, G++ > 5
- Install isolate
- Build as `cmake . && make`
- Run `./basic-worker`
