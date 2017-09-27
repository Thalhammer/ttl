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
