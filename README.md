
![CoCADa](./img/cocada-large.png)

# **CO**cada **C**ollection of **A**lgorithms and **DA**ta structures

## What is COCADA?

COCADA is a C programming library, or rather a collection of libraries. It is composed of assorted things that I have written and used in my personal projects, and decided to start getting organised. COCADA is intended mainly for the author's educational and academic work. Although it can be used for any purpose, it has not been thoroughly tested neither it comes with any guarantees. It is also a perpetual work in progress. 

## What about the name?

First and foremost, COCADA is a traditional sweet from the Northeast of Brazil, made of shredded coconut, sugar and milk. It is also a configurable acronym. In its default left-recursive form, it means **CO**cada **C**ollection of **A**lgorithms and **DA**ta structures. The initial CO can be changed for C-Only, COol, or any other adjective you may see fit. :)

## What is in the library?

COCADA is composed of interdependent sub-libraries or modules. Currently, they are

- **libcocada** This is the basic stuff upon which the rest is constructed. It contains convenience abstractions over low-level C-language constructs, minimal infrastructure for the object model used by most other modules, common ADT for dynamic collections such as vectors, hashtables and trees, and support for I/O and user interaction
- **libcocadastrproc** String-processing (matching, indexing) data structures and algorithms
- **libcocadabio** Bioinformatics-related stuff
- **libcocadasketch** Data Sketches (mainly) for big data streaming applications

The dependency graph is as follows

```
libcocadasketh
  |
  |             libcocadabio
  |              |       |
  |              v       |
  |  libcocadastrproc    |
   \    \               /
    \    \             /
     \___ \__    _____/
         |   |  |
         v   v  v  
        libcocada
```

I plan to keep the libraries free from cyclic dependencies.


## How to use it

COCADA libraries can be used in a few different ways. First you can just copy the source files and use them in a separate project, as long as you respect the dependencies above. Alternatively - and perhaps more appropriately - they can be compiled and used as either  [static](http://www.tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html)  or  [shared](http://www.tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html) libraries.

#### Installing COCADA as a static library

Each COCADA library has a separate Makefile. In order to build the static library, `cd` to the library directory and run

```
$ make staticlib
```

This will compile the source files and create one static library for each library in the dependency path. The static libraries `.a` files will be put into the `build/lib/static` subdir. For example, if you do

```
$ cd libcocadabio
$ make staticlib
```

Then the libcocadabio could look like something as

```
libcocadabio/
├── build
│   ├── debug/ 
│   ├── lib
│   │   ├── shared/
│   │   └── static
│   │       ├── include
│   │       │   ├── alphabet.h
│   │       │   ├── arrays.h
│   │       │   ... 
│   │       ├── libcocada.a
│   │       ├── libcocadabio.a
│   │       └── libcocadastrproc.a
│   └── release/
│   Makefile
├── src/
...
```
Notice in particular the three `.a` files and the `include/` subdir that will contain all the header files used in these three libraries.

By default this static library file will be copied to the`/usr/local/lib/cocada`, and the header (`.h`) files will be copied to `/usr/local/include/cocada`.

> NOTE: You will need writing permissions to the system directory `/usr/local/` which means you will likely need to run make via [sudo](http://www.sudo.ws/man/sudo.man.html).


#### Installing COCADA as a shared library

To build COCADA as a *shared* library, simply run

```
$ sudo make install-shared
```

This will compile the source files and create a library file named `libcocada.so.x.y.z`, where `x.y.z` corresponds to the version number in the form [*major.minor.release*](http://www.tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html#AEN49). By default this shared library file will be copied to the `/usr/local/lib/cocada` directory, and the header (`.h`) files will be copied to `/usr/local/include/cocada`. In addition to that, the installation script will run [`ldconfig`](https://linux.die.net/man/8/ldconfig) to update the system links to the library.


#### Installing to different directories

You can change the default library and/or header files destination directories, for both the static and shared versions, by providing the desired values to the `INSTALL_DIR` and `INCLUDE_DIR` make variables respectively.

For example, if you want to install the shared library file to `foo`, and the header files to be copied to `bar`, then you should run the command

```
$ sudo make install-shared INSTALL_DIR=foo INCLUDE_DIR=bar
```


