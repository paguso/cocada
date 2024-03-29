
![CoCADa](./img/cocada-large.png)

# **CO**cada **C**ollection of **A**lgorithms and **DA**ta structures

## What is COCADA?

COCADA is a C programming collection of libraries. It is composed of assorted things that I have written and used in my personal projects. COCADA is intended mainly for the author's educational and academic work, it has not been thoroughly tested, and it comes with no guarantees. It is also a perpetual work in progress. 

## What about the name?

First and foremost, COCADA is a traditional sweet from the Northeast of Brazil, made of shredded coconut, sugar and milk. It is also a configurable acronym. In its default left-recursive form, it means **CO**cada **C**ollection of **A**lgorithms and **DA**ta structures. The initial CO can be changed for C-Only, COol, or any other adjective you may see fit. :)

## What is in the library?

COCADA is composed of interdependent sub-libraries or modules. Currently, they are

- **libcocada** is the basic stuff upon which the rest is constructed. It contains convenience abstractions over low-level C-language constructs, minimal infrastructure for the object model used by most other modules, common ADT for dynamic collections such as vectors, hashtables and trees, and support for I/O and user interaction
- **libcocadaapp** Things for building apps, like a CLI parser
- **libcocadastrproc** contains string-processing (matching, indexing) data structures and algorithms
- **libcocadabio** contains Bioinformatics-related stuff
- **libcocadasketch** contains Data Sketches (mainly) for big data streaming applications

The dependency graph is as follows

```
cocadaapp  cocadasketch    cocadabio
  |              |         |       |
  |              |         v       |
  |              |  cocadastrproc  |
   \             |     /          /
    \            |    /          /
     \________   |   /  ________/
              |  |  |  |
              v  v  v  v
                cocada
```

I plan to keep the libraries free from cyclic dependencies.


## How to use it

COCADA libraries can be used in a few different ways. First you can just clone the source files and use them in a separate project, as long as you respect the dependencies above. Alternatively they can be compiled and used as either  [static](http://www.tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html)  or  [shared](http://www.tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html) libraries.

COCADA uses GNU Make as a build and installation manager. Running 
```
make help
```
in the COCADA root directory will give you a brief help on the available options. 
Generally speaking, running
```
make <target> <libraries> 
```
will perform the given `<target>` in the given list of `<libraries>`. You can input `all` instead of a list of library names to make the target on all libraries.


### Copying COCADA sources

The most straightforward way of using COCADA in your project is by copying the source files into your project's source directory. This can be acomplished by running
```
make clone <libraries> clone_dest=<destination>
``` 
This will copy the header and source files of the given libraries to the given `<destination>` directory (default=`~/cocada`). By default, and for simplicity, the copy is "flat", that is, all the files will be put directly under the `<destination>` directory. However, if you want to reproduce the source directory structure of the original COCADA repositories, you can define the variable `clone_tree` by entering the option `-e clone_tree=true`.

Finally, it is important to notice that cloning is recursive. That is, when you clone a given library, not only its header and source files get copyied, but also those of all libraries upon which it depends.


### Installing COCADA as a static library

In order to build and install a COCADA static library, we just have to `cd` to the library directory and run

```
$ make staticlib_build <libraries>
$ make staticlib_install <libraries>
```

This will compile the source files and create one static archive for each library in the dependency closure. Generally speaking, when a library `A` depends on library `B`, both libraries will be built and installed (almost) independently. It is important to understand how exactly this is done in order to use and update the libraries correctly.

Let's take for example the `codadabio` library. This library depends on `cocadastrproc` and `cocada`, with `cocadastrproc` also depending itself on `cocada`, and therefore the dependency closure consists of these three libraries.  Thus if we run

```
$ make staticlib_build cocadabio
```

we would end up with something like this

```
...
├── libcocada
│   ├── build
│   │   ├── lib.deps
│   │   └── static
│   │       ├── ...
│   │       ├── libcocada.a
│   │       └── ...
│   ├── Makefile
│   ├── src/
│   └── test/
...
├── libcocadabio
│   ├── build
│   │   ├── lib.deps
│   │   └── static
│   │       ├── ...
│   │       └── libcocadabio.a
│   ├── Makefile
│   ├── src/
│   └── test/
...
├── libcocadastrproc
│   ├── build
│   │   ├── lib.deps
│   │   └── static
│   │       ├── ...
│   │       ├── libcocadastrproc.a
│   │       └── ...
│   ├── Makefile
│   ├── src/
│   └── test/
...

```
Notice that all the three libraries in the dependency closure were compiled for `release`, and the archives `libcocada.a`, `libcocadastrproc.a` and `libcocadabio.a` were created from the object files, and each put in the `build` subdirectory of the corresponding library.

After that, by running

```
$ make staticlib_install cocadabio
```
the just-built libraries are copied to `/use/local/lib` (or another directory given to the makefile via the `static_install_dir` variable - run `make help` for more instructions). In addition to that, the `.h` header files of each library are copied to a separate subirectory inside `usr/local/include`, named after the library. 

> NOTE: You will need writing permissions to the system directory `/usr/local/lib` which means that you might need to run the installation as [sudo](http://www.sudo.ws/man/sudo.man.html).

This modular installation procedure results in all sublibraries in the dependency closure to be independently available for use as a static library. However this also implies that if we update one of the libraries later, this will likely break or cause inconsitencies with the other installed libraries that depend on it. In our example, if we later update the `cocadastrproc` library, this will trigger the update of the `cocada` base library, but **not** the `cocadabio` library. It is therefore advisable to update **all** installed cocada libraries at once.


### Installing COCADA as a shared library

Building and installing COCADA as a *shared* library is acomplished by running

```
$ make sharedlib_build <libraries>
$ make sharedlib_install <libraries>
```

This will compile the source files and create a library file named `lib<lib_name>/build/shared/lib<lib_name>.so.x.y.z`, where `x.y.z` corresponds to the version number in the form [*major.minor.release*](http://www.tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html#AEN49) for each library in the dependency closure. By default, the installation process will copy these shared library files to `/usr/local/lib` directory, and the header (`.h`) files will be copied to `/usr/local/include/<lib_name>` as with the static libraries. In addition to that, the installation script will run [`ldconfig`](https://linux.die.net/man/8/ldconfig) to update the system links to the library.


### Installing to different directories

You can change the default library and/or header files destination directories, for both the static and shared versions, by providing the desired values to the `install_dir` and `include_dir` make variables respectively.

For example, if you want to install the shared library file to `foo`, and the header files to be copied to `bar`, then you should run the command

```
$ make sharedlib_install install_dir=foo include_dir=bar
```


