# mbs [![Build Status](https://img.shields.io/travis/laserpants/mbs/master.svg?style=flat)](https://travis-ci.org/laserpants/mbs)

A simple command line tool to keep track of the amount of data ("em-bees") sent 
and received over a network interface. Useful to monitor data usage against a 
pre-paid data bundle or some fixed usage limit.

### Building

```bash
./configure
make
make install
```

where, normally, the last command is invoked as `sudo make install`, since it 
requires root privileges. 

### Usage

```
mbs [-vk] [--help] [--version] [--ascii] [-a <amount>] [<interface>]
```

#### Examples

Specify the amount of data available using the `--available` (`-a`) flag to run
the command in countdown mode. 

```
mbs -a 100M
```

#### Flags

| Flag           | Short option | Description                             |
|----------------|--------------|-----------------------------------------|
| --help         |              | Display help and exit.                  |   
| --version      |              | Display version info and exit.          |   
| --verbose      | -v           | Verbose output                          |   
| --ascii        |              | Do not use Unicode characters in output |   
| --keep-running | -k           | Do not exit when data limit exceeded or connection is lost |   
| --available    | -a           | Data available to use in your subscription plan or budget  |   

