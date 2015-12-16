# BasicWorker

[![Join the chat at https://gitter.im/ReCodEx/BasicWorker](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ReCodEx/BasicWorker?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/ReCodEx/BasicWorker.svg?branch=master)](https://travis-ci.org/ReCodEx/BasicWorker)

A daemon that consumes assignments from the message queue, builds and runs them 
and then sends results back.

## How to run it

- Install dependencies according to [common](https://github.com/ReCodEx/GlobalWiki/wiki/System-configuration#common) and [worker-specific](https://github.com/ReCodEx/GlobalWiki/wiki/System-configuration#worker) configuration
- Download `zmq.hpp` and Google test framework using `git submodule update --init`
- Build with cmake: `mkdir build && cd build && cmake .. && make`
- Run `./basic-worker`
