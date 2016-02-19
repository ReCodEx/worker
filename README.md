# BasicWorker

[![Join the chat at https://gitter.im/ReCodEx/worker](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ReCodEx/worker?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Linux build Status](https://img.shields.io/travis/ReCodEx/worker/master.svg?label=Linux%20build)](https://travis-ci.org/ReCodEx/worker)
[![Windows build status](https://img.shields.io/appveyor/ci/ReCodEx/BasicWorker/master.svg?label=Windows%20build)](https://ci.appveyor.com/project/ReCodEx/basicworker/branch/master)

A daemon that consumes assignments from the message queue, builds and runs them 
and then sends results back.

## How to run it

### Linux
- Install dependencies according to [common](https://github.com/ReCodEx/GlobalWiki/wiki/System-configuration#common) and [worker-specific](https://github.com/ReCodEx/GlobalWiki/wiki/System-configuration#worker) configuration
- Download dependencies using `git submodule update --init`
- Build with cmake: `mkdir build && cd build && cmake .. && make`
- Run `./basic-worker` (or unit tests `./tests/run_tests`)

### Windows
- Look at description in our wiki over [here](https://github.com/ReCodEx/GlobalWiki/wiki/System-configuration#windows-worker).
