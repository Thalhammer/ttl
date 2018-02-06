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

#### any ####
A class similar to std::any but with some additional features like [advanced type information](#type) and casting to base classes.

#### config ####
Read a name-value config file supporting including files.

#### contract[wip] ####
Contract based programming.

#### dynlib ####
Open a dynamic loadable library (dll/so) and get method addresses.

#### linq ####
Query containers in a linq like query.

#### logger ####
Threadsafe std::cout like logger implementation.

#### noncopyable ####
Inherit to disable copy and assignment construction.

#### noop_mutex ####
A class compatible to std::mutex that does nothing.

#### process ####
Open a process and read/write stdin, stdout, stderr.

#### signal ####
Allows to wire up callbacks and handle events in a easy and intuitive way.

#### string_util ####
Helper functions for std::string manipulation:
* rtrim, rtrim_copy, ltrim, ltrim_copy, trim, trim_copy
* split, join
* starts_with, ends_with

#### timer ####
Schedule a task at a specified point in time or in a fixed interval.

#### type ####
A class to allow passing around information about a type (basicly everything in <type_traits> but at runtime).

#### binary_reader and binary_writer ####
A class used to write and read binary data into/from a stream oriented on C#'s BinaryWriter.

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

## Experimental Methods ##
Some of the provided methods rely on non standard features or compiler internals. Even thought I do my best to test and improve them they might one day fail in a big boom (Segfault, heap corruption, etc.).
* any::upcast<br>
  Should be safe on linux because there is at least a [standard](https://itanium-cxx-abi.github.io/cxx-abi/), but uses compiler internals on windows.
* dynlib::get_symbols<br>
  Parses ELF/PE signature, should work most of the time but is not guaranteed to do so.
  
  