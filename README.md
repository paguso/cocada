
![CoCADa](./img/cocada-large.png)

# **CO**cada **C**ollection of **A**lgorithms and **DA**ta structures


## Installation

### Linux installation

The COCADA repository contains a `Makefile` located at the root directory, which we will refer to as `cocada-root`.

The COCADA library is written in pure C and can be built and installed using that Makefile either as a [static](http://www.tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html)  or as a [shared](http://www.tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html) library.

#### Installing COCADA as a static library

To build COCADA as a *static* library, simply run

```
$ sudo make install-static 
```

This will compile the source files and create a library file named `libcocada.a`. By default this static library file will be copied to the`/usr/local/lib/cocada`, and the header (`.h`) files will be copied to `/usr/local/include/cocada`.

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


