# mbs [![Build Status](https://img.shields.io/travis/laserpants/mbs/master.svg?style=flat)](https://travis-ci.org/laserpants/mbs)

A simple command line tool to keep track of the amount of data ("em-bees") sent 
and received over a network interface. Useful to monitor data usage against a 
pre-paid data bundle or some fixed usage limit.

### Building

To build the executable, run

```bash
./configure
make
make install
```

where `make install` usually requires root privileges and is invoked with 
`sudo`. 

### Usage

```
mbs [-vk] [--help] [--version] [--ascii] [-a <amount>] [<interface>]
```

If no `<interface>` is given, the program will try to automatically find a 
running network interface (excluding `lo`).

#### Examples

Specify the amount of data available using the `--available` (`-a`) flag to run
the command in countdown mode. The following command specifies a data limit of
300 KB.

```
mbs -a 300K
```

![mbs](https://github.com/laserpants/mbs/blob/master/mbs.gif)

By default, the command will exit once the data limit is reached, or if the
connection is lost. Use the `--keep-running` flag to modify this behavior.

```
mbs -a 10K --keep-running
```

#### Flags

| Flag             | Short option   | Description                             |
|------------------|----------------|-----------------------------------------|
| `--help`         |                | Display help and exit.                  |   
| `--version`      |                | Display version info and exit.          |   
| `--verbose`      | `-v`           | Render verbose output.                  |   
| `--ascii`        |                | Disable Unicode characters.             |   
| `--keep-running` | `-k`           | Do not exit when data limit exceeded or connection is lost.          |   
| `--available`    | `-a`           | Amount of data available to use in your subscription plan or budget. |   

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


