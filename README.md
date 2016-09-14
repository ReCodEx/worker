# Worker

[![Linux build Status](https://img.shields.io/travis/ReCodEx/worker/master.svg?label=Linux%20build)](https://travis-ci.org/ReCodEx/worker)
[![Windows build status](https://img.shields.io/appveyor/ci/ReCodEx/BasicWorker/master.svg?label=Windows%20build)](https://ci.appveyor.com/project/ReCodEx/basicworker/branch/master)
[![License](http://img.shields.io/:license-mit-blue.svg)](http://badges.mit-license.org)
[![Docs](https://img.shields.io/badge/docs-latest-brightgreen.svg)](http://recodex.github.io/worker/)
[![Wiki](https://img.shields.io/badge/docs-wiki-orange.svg)](https://github.com/ReCodEx/GlobalWiki/wiki)

A daemon that consumes assignments from the message queue, builds and runs them 
and then sends results back.

## How to run it

### Linux
- Install dependencies according to [common](https://github.com/ReCodEx/GlobalWiki/wiki/Build-and-Deployment#common) and [worker-specific](https://github.com/ReCodEx/GlobalWiki/wiki/Build-and-Deployment#worker) configuration
- Download dependencies using `git submodule update --init`
- Build with cmake: `mkdir build && cd build && cmake .. && make`
- Run `./basic-worker` (or unit tests `./tests/run_tests`)

### Windows
- Look at description in our wiki over [here](https://github.com/ReCodEx/GlobalWiki/wiki/Build-and-Deployment#windows-worker).
