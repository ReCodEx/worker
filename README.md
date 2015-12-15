# BasicWorker

[![Join the chat at https://gitter.im/ReCodEx/BasicWorker](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ReCodEx/BasicWorker?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/ReCodEx/BasicWorker.svg?branch=master)](https://travis-ci.org/ReCodEx/BasicWorker)

A daemon that consumes assignments from the message queue, builds and runs them 
and then sends results back.

## How to run it

- Install ZeroMQ, version at least 4.0. Package name should be `libzmq3-dev`
- Install dependencies with `git submodule update --init`
- Install CMake, GNU Make, G++ > 5
- Install isolate
- Install libcurl
- Build as `cmake . && make`
- Run `./basic-worker`
