# ttl
Thalhammer Template Library

This is a header only library containing usable classes for developing apps in C++.
Major goals are:
* Header only
* Selfcontained (no external libraries required)
* Support both Windows and Linux
* Fully unit tested

It supports C++11 and newer and works on (at least) g++ and MSVC 2015 Update 2.
Currently it provides the following classes:

#### config ####
Read a name-value config file supporting including files.

#### dynlib ####
Open a dynamic loadable library (dll/so) and get method addresses.

#### logger ####
Threadsafe std::cout like logger implementation.

#### process ####
Open a process and read/write stdin, stdout, stderr.

#### string_util ####
Helper functions for std::string manipulation:
* rtrim, rtrim_copy, ltrim, ltrim_copy, trim, trim_copy
* split, join
* starts_with, ends_with

#### timer ####
Schedule a task at a specified point in time or in a fixed interval.

#### noncopyable ####
Inherit to disable copy and assignment construction.

#### linq ####
Query containers in a linq like query.

#### signal ####
Allows to wire up callbacks and handle events in a easy and intuitive way.

#### contract[wip] ####
Contract based programming.

## Building ##

Since ttl is a header only library there is not really something to build.
You can however build and execute the Testcases to check if there are any bugs in ttl on your system configuration.
```
make -C ttl
./ttl/test
```
If you experience a build failure or one of the tests fails please create a issue to help me solve the problem.
Feel free to create a pullrequest to add features or fix bugs.
#### Building a debian package ####
You can also create a debian package.
```
make -C ttl package
```
This will create a debian package named libttl_\<version\>.deb within the ttl subfolder.