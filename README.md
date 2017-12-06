# mbs [![Build Status](https://img.shields.io/travis/laserpants/mbs/master.svg?style=flat)](https://travis-ci.org/laserpants/mbs)

A simple command line tool to keep track of the amount of data ("em-bees") sent 
and received over a network interface. Useful to monitor data usage against a 
pre-paid data bundle or some fixed usage limit.

To install

```bash
./configure
make
make install
```

where, normally, the last command is invoked as `sudo make install`, since it requires root privileges. 
