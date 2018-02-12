# mbs [![Build Status](https://img.shields.io/travis/laserpants/mbs/master.svg?style=flat)](https://travis-ci.org/laserpants/mbs)

A command line tool to keep track of the amount of data sent and received over 
a network interface. This can be convenient for monitoring data usage against a 
pre-paid bundle or some other fixed usage limit.

### Building

To build the executable and tests, run

```bash
./configure
make
make install
```

where `make install` requires root privileges and is usually invoked 
with `sudo`. 

### Usage

```
mbs [-vkp] [--help] [--version] [--ascii] [-a <amount>] [--statsfile=<path>] [<interface>]
```

If no `<interface>` is given, the program will try to automatically find an 
active network interface (excluding `lo`).

#### Examples

Set the amount of data available using the `--available` (`-a`) flag to run
the command in *countdown* mode. The following example specifies a data limit 
of 300 KB.

```
mbs -a 300K
```

![mbs](https://github.com/laserpants/mbs/blob/master/mbs.gif)

To tell the command to monitor the network interface `wlan0`:

```
mbs -a 300K wlan0
```

By default, the command will exit once the usage limit is reached, or if the 
connection is lost. Use the `--keep-running` flag to modify this behavior.

```
mbs -a 10K --keep-running
```

You can also omit the `--available` flag, in which case the command will 
run in a simplified mode&mdash;only showing the amount of data used since it 
started.

![mbs](https://raw.githubusercontent.com/laserpants/mbs/master/mbs2.gif)

If the `--ascii` flag is provided, only characters in the (extended) ASCII 
character set are used in the interface:

![mbs](https://raw.githubusercontent.com/laserpants/mbs/master/mbs3.gif)

#### Persistent sessions

When the command is run with the `--persistent` (`-p`) flag, it will try to 
continue from where the last session ended. It does so by reading the last 
saved state (sent and received bytes count) from a *stats* file. Note that this 
will not work if the kernel's TX/RX counters were reset since the last time the 
command was run (e.g., after a system reboot).

The stats file's location can be set using the `--statsfile=<path>` flag. If 
this flag is not provided, then `$HOME/.mbs` is used as a default.

### Flags

| Flag             | Short option   | Description                             |
|------------------|----------------|-----------------------------------------|
| `--help`         |                | Display help and exit.                  |
| `--version`      |                | Display version info and exit.          |
| `--verbose`      | `-v`           | Render verbose output.                  |
| `--ascii`        |                | Disable non-ascii Unicode characters.   |
| `--keep-running` | `-k`           | Do not exit when data limit is exceeded or connection is lost. |
| `--persistent`   | `-p`           | Continue from where last session ended. (See [Persistent sessions](https://github.com/laserpants/mbs#persistent-sessions).) |
| `--available`    | `-a`           | Amount of data available to use in your subscription plan or budget. |
| `--statsfile`    |                | Override default stats file path. (See [Persistent sessions](https://github.com/laserpants/mbs#persistent-sessions).) |

The `--available` argument accepts the following suffixes:

| Suffix                     | Unit                   | Size             |
|----------------------------|------------------------|------------------|
| **B**, **b**, or no suffix | Bytes                  | 1                |
| **kB**, or **k**           | Kilobyte               | 1000             |
| **KB**, **K**, or **KiB**  | Kibibyte<sup>†</sup>   | 2<sup>10</sup>   |
| **mB**, or **m**           | Megabyte               | 1000<sup>2</sup> |
| **MB**, **M**, or **MiB**  | Mebibyte<sup>†</sup>   | 2<sup>20</sup>   |
| **gB**, or **g**           | Gigabyte               | 1000<sup>3</sup> |
| **GB**, **G**, or **GiB**  | Gibibyte<sup>†</sup>   | 2<sup>30</sup>   |

†) Defined by the International Electrotechnical Commission (IEC).

A decimal point can also be used; e.g., `mbs -a 100.5M`.

### Third-party libraries

This repository includes [Argtable3](http://www.argtable.org/) as a git submodule. The Argtable3 library is copyright (c) 1998-2001, 2003-2011 Stewart Heitmann and licensed under the 3-Clause BSD License.
