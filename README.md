# Worker

[![Linux build Status](https://github.com/ReCodEx/worker/workflows/Linux%20Build/badge.svg)](https://github.com/ReCodEx/worker/actions)
[![Windows build status](https://img.shields.io/appveyor/ci/Neloop/worker/master.svg?label=Windows%20build)](https://ci.appveyor.com/project/Neloop/worker/branch/master)
[![codecov](https://codecov.io/gh/ReCodEx/worker/branch/master/graph/badge.svg?token=AYHQA9R8PJ)](https://codecov.io/gh/ReCodEx/worker)
[![License](http://img.shields.io/:license-mit-blue.svg)](http://badges.mit-license.org)
[![Docs](https://img.shields.io/badge/docs-latest-brightgreen.svg)](http://recodex.github.io/worker/)
[![Wiki](https://img.shields.io/badge/docs-wiki-orange.svg)](https://github.com/ReCodEx/wiki/wiki)
[![GitHub release](https://img.shields.io/github/release/recodex/worker.svg)](https://github.com/ReCodEx/wiki/wiki/Changelog)
[![COPR](https://copr.fedorainfracloud.org/coprs/semai/ReCodEx/package/recodex-worker/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/semai/ReCodEx/)

The job of the worker is to securely execute a job according to its
configuration and upload results back for latter processing. After receiving an
evaluation request, worker has to do following:

- download the archive containing submitted source files and configuration file
- download any supplementary files based on the configuration file, such as test 
  inputs or helper programs (this is done on demand, using a `fetch` command
  in the assignment configuration)
- evaluate the submission according to job configuration
- during evaluation progress messages can be sent back to broker
- upload the results of the evaluation to the fileserver
- notify broker that the evaluation finished

## Installation

### COPR Installation

Follows description for CentOS which will do all steps as described in _Manual Installation_.

```
# yum install yum-plugin-copr
# yum copr enable semai/ReCodEx
# yum install recodex-worker
```

### Manual Installation

#### Dependencies

Worker specific requirements are written in this section. It covers only basic
requirements, additional runtimes or tools may be needed depending on type of
use. The package names are for CentOS if not specified otherwise. 

- Boost 1.66 development libs (`boost-devel` package)
- ZeroMQ in version at least 4.0, packages `zeromq` and `zeromq-devel`
  (`libzmq3-dev` on Debian)
- YAML-CPP library, `yaml-cpp` and `yaml-cpp-devel` (`libyaml-cpp0.5v5` and
  `libyaml-cpp-dev` on Debian)
- libcurl library `libcurl-devel` (`libcurl4-gnutls-dev` on Debian)
- libarchive library as optional dependency. Installing will speed up build
  process, otherwise libarchive is built from source during installation.
  Package name is `libarchive` and `libarchive-devel` (`libarchive-dev` on
  Debian)

**Isolate** (only for Linux installations)

First, we need to compile sandbox Isolate from source and install it. Current
worker is tested against version 1.3, so this version needs to be checked out.
Assume that we keep source code in `/opt/src` dir. For building man page you
need to have package `asciidoc` installed.

```
$ cd /opt/src
$ git clone https://github.com/ioi/isolate.git
$ cd isolate
$ git checkout v1.3
$ make
# make install && make install-doc
```

For proper work Isolate depends on several advanced features of the Linux
kernel. Make sure that your kernel is compiled with `CONFIG_PID_NS`,
`CONFIG_IPC_NS`, `CONFIG_NET_NS`, `CONFIG_CPUSETS`, `CONFIG_CGROUP_CPUACCT`,
`CONFIG_MEMCG`. If your machine has swap enabled, also check
`CONFIG_MEMCG_SWAP`. With which flags was your kernel compiled with can be found
in `/boot` directory, file `config-` and version of your kernel. Red Hat based
distributions should have these enabled by default, for Debian you you may want
to add the parameters `cgroup_enable=memory swapaccount=1` to the kernel
command-line, which can be set by adding value `GRUB_CMDLINE_LINUX_DEFAULT` to
`/etc/default/grub` file.

For better reproducibility of results, some kernel parameters can be tweaked:

- Disable address space randomization. Create file
  `/etc/sysctl.d/10-recodex.conf` with content `kernel.randomize_va_space=0`.
  Changes will take effect after restart or run `sysctl
  kernel.randomize_va_space=0` command.
- Disable dynamic CPU frequency scaling. This requires setting the cpufreq
  scaling governor to _performance_.

#### Clone worker source code repository

```
$ git clone https://github.com/ReCodEx/worker.git
$ git submodule update --init
```

#### Install worker on Linux

It is supposed that your current working directory is that one with cloned
worker source codes.

- Prepare environment running `mkdir build && cd build`
- Build sources by `cmake ..` following by `make`
- Build binary package by `make package` (may require root permissions).  Note
  that `rpm` and `deb` packages are build in the same time. You may need to have
  `rpmbuild` command (usually as `rpmbuild` or `rpm` package) or edit
  CPACK_GENERATOR variable in _CMakeLists.txt_ file in root of source code tree.
- Install generated package through your package manager (`yum`, `dnf`, `dpkg`).

The worker installation process is composed of following steps:

- create config file `/etc/recodex/worker/config-1.yml`
- create systemd unit file `/etc/systemd/system/recodex-worker@.service`
- put main binary to `/usr/bin/recodex-worker`
- put judges binaries to `/usr/bin/` directory
- create system user and group `recodex` with `/sbin/nologin` shell (if not
  already existing)
- create log directory `/var/log/recodex`
- set ownership of config (`/etc/recodex`) and log (`/var/log/recodex`)
  directories to `recodex` user and group

_Note:_ If you do not want to generate binary packages, you can just install the
project with `make install` (as root). But installation through your
distribution's package manager is preferred way to keep your system clean and
manageable in long term horizon.

#### Install worker on Windows

There are basically two main dependencies needed, **Windows 7** or higher and
**Visual Studio 2015+**. Provided simple installation batch script should do all
the work on Windows machine. Officially only VS2015 and 32-bit compilation is
supported, because of hardcoded compile options in installation script. If
different VS or different platform is needed, the script should be changed to
appropriate values.

Mentioned script is placed in *install* directory alongside supportive scripts
for UNIX systems and is named *win-build.cmd*. Provided script will do almost
all the work connected with building and dependency resolving (using
**NuGet** package manager and `msbuild` building system). Script should be
run under 32-bit version of _Developer Command Prompt for VS2015_ and from
*install* directory.

Building and installing of worker is then quite simple, script has command line
parameters which can be used to specify what will be done:

- *-build* -- It is the default options if none specified. Builds worker and its
  tests, all is saved in *build* folder and subfolders.
- *-clean* -- Cleanup of downloaded NuGet packages and built
  application/libraries.
- *-test* -- Build worker and run tests on compiled test cases.
- *-package* -- Generation of clickable installation using cpack and
  [NSIS](http://nsis.sourceforge.net/) (has to be installed on machine to get
  this to work).

```
install> win-build.cmd  # same as: win-build.cmd -build
install> win-build.cmd -clean
install> win-build.cmd -test
install> win-build.cmd -package
```

All build binaries and cmake temporary files can be found in *build* folder,
classically there will be subfolder *Release* which will contain compiled
application with all needed dlls. Once if clickable installation binary is
created, it can be found in *build* folder under name 
*recodex-worker-VERSION-win32.exe*. Sample screenshot can be found on following
picture.

#### Usage

A systemd unit file is distributed with the worker to simplify its launch. It 
integrates worker nicely into your Linux system and allows you to run it 
automatically on system startup. It is possible to have more than one worker on 
every server, so the provided unit file is a template. Each instance of the 
worker unit has a unique string identifier, which is used for managing that 
instance through systemd. By default, only one worker instance is ready to use 
after installation and its ID is "1".

- Starting worker with id "1" can be done this way:
```
# systemctl start recodex-worker@1.service
```
Check with
```
# systemctl status recodex-worker@1.service
```
if the worker is running. You should see "active (running)" message.

- Worker can be stopped or restarted accordingly using `systemctl stop` and
  `systemctl restart` commands.
- If you want to run worker after system startup, run:
```
# systemctl enable recodex-worker@1.service
```
For further information about using systemd please refer to systemd
documentation.

##### Adding new worker

To add a new worker you need to do a few steps:

- Make up an unique string ID.
- Copy default configuration file `/etc/recodex/worker/config-1.yml` to the same
  directory and name it `config-<your_unique_ID>.yml`
- Edit that config file to fit your needs. Note that you must at least change
  _worker-id_ and _logger file_ values to be unique.
- Run new instance using
```
# systemctl start recodex-worker@<your_unique_ID>.service
```

## Configuration

Worker have a default configuration which is applied to worker itself or is used
in given jobs (implicitly if something is missing, or explicitly with special
variables). This configuration is hardcoded in worker sources and can be
rewritten by explicitly declared configuration file. Format of this
configuration is YAML file with similar structure as job configuration. The
default location is `/etc/recodex/worker/config-<N>.yml` where `N` is identifier
of the particular worker instance.

Example configuration file can be found in `examples/config.yml`.

### Configuration items

- **worker-id** -- unique identification of worker at one server. This id is
  used by _isolate_ sandbox on linux systems, so make sure to meet requirements
  of the isolate (default is number from 1 to 999).
- _worker-description_ -- human readable description of this worker
- **broker-uri** -- URI of the broker (hostname, IP address, including port,
  ...)
- _broker-ping-interval_ -- time interval how often to send ping messages to
  broker. Used units are milliseconds.
- _max-broker-liveness_ -- specifies how many pings in a row can broker miss
  without making the worker dead.
- _headers_ -- map of headers specifies worker's capabilities
	- _env_ -- list of environmental variables which are sent to broker in init
	  command
	- _threads_ -- information about available threads for this worker
- **hwgroup** -- hardware group of this worker. Hardware group must specify
  worker hardware and software capabilities and it is main item for broker
  routing decisions.
- _working-directory_ -- where will be stored all needed files. Can be the same
  for multiple workers on one server.
- **file-managers** -- addresses and credentials to all file managers used (eq.
  all different frontends using this worker)
	- **hostname** -- URI of file manager
	- _username_ -- username for http authentication (if needed)
	- _password_ -- password for http authentication (if needed)
- _file-cache_ -- configuration of caching feature
	- _cache-dir_ -- path to caching directory. Can be the same for multiple
	  workers.
- _logger_ -- settings of logging capabilities
	- _file_ -- path to the logging file with name without suffix.
	  `/var/log/recodex/worker` item will produce `worker.log`, `worker.1.log`,
	  ...
	- _level_ -- level of logging, one of `off`, `emerg`, `alert`, `critical`,
	  `err`, `warn`, `notice`, `info` and `debug`
	- _max-size_ -- maximal size of log file before rotating in bytes
	- _rotations_ -- number of rotation kept
- _limits_ -- default sandbox limits for this worker. All items are described in
  assignments section in job configuration description. If some limits are not
  set in job configuration, defaults from worker config will be used. In such
  case the worker's defaults will be set as the maximum for the job. Also,
  limits in job configuration cannot exceed limits from worker.
- **max-output-length** -- used for `tasks.{task}.sandbox.output` option, defined
  in bytes, applied to both stdout and stderr and is not divided, both will get
  this value
- **max-carboncopy-length** -- used for `tasks.{task}.sandbox.carboncopy-stdout`
  and `tasks.{task}.sandbox.carboncopy-stderr` options, specifies maximal length
  of the files which will be copied, defined in bytes
- **cleanup-submission** -- if set to true, then files produced during evaluation
  of submission will be deleted at the end, extra caution is advised because this
  setup can cause extensive disk usage

### Isolate sandbox

New feature of the 1.3 version is a possibility of limit Isolate box to one or
more CPU or memory nodes. This functionality is provided by _cpusets_ kernel
mechanism and is now integrated into Isolate. It is allowed to set only
`cpuset.cpus` and `cpuset.mems` which should be just fine for sandbox purposes.
As a kernel functionality further description can be found in manual page of
_cpuset_ or in Linux documentation in section
`linux/Documentation/cgroups/cpusets.txt`. As previously stated this settings
can be applied for particular Isolate boxes and has to be written in Isolate
configuration. Standard configuration path should be `/usr/local/etc/isolate`
but it may depend on your installation process.  Configuration of _cpuset_ in
there is described in example below.

```
box0.cpus = 0  # assign processor with ID 0 to isolate box with ID 0
box0.mems = 0  # assign memory node with ID 0
# if not set, linux by itself will decide where should
# the sandboxed programs run at
box2.cpus = 1-3  # assign range of processors to isolate box 2
box2.mems = 4-7  # assign range of memory nodes 
box3.cpus = 1,2,3  # assign list of processors to isolate box 3
```

- _cpuset.cpus:_ Cpus limitation will restrict sandboxed program only to
  processor threads set in configuration. On hyperthreaded processors this means
  that all virtual threads are assignable, not only the physical ones. Value can
  be represented by single number, list of numbers separated by commas or range
  with hyphen delimiter.
- _cpuset.mems:_ This value is particularly handy on NUMA systems which has
  several memory nodes. On standard desktop computers this value should always
  be zero because only one independent memory node is present. As stated in
  `cpus` limitation there can be single value, list of values separated by comma
  or range stated with hyphen.

## Documentation

Feel free to read the documentation on [our wiki](https://github.com/ReCodEx/wiki/wiki).
