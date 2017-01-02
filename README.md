# RIVET

Program to visualize two-parameter persistent homology. 
Designed by Michael Lesnick and Matthew Wright. 
Created December 2013.  

## Contributors
Michael Lesnick (Princeton)
Matthew Wright (St. Olaf College)
Bryn Keller (Intel Labs)

TODO: Other contributors please add yourselves!

## Requirements

Before starting to build RIVET, make sure you have the following installed:
 
* A C++ compiler (g++ or clang are what we use)
* CMake
* Qt 5
* Boost (including boost serialization; version 1.60 or newer required)

All of these are generally available using your operating systems's package
manager. For example, on Mac OS X:
    
    brew install cmake qt5 boost
    
On Ubuntu:

    sudo apt-get install cmake qt5-default qt5-qmake qtbase5-dev-tools libboost-all-dev

## Building
When running qmake in the steps below, make sure to use the qmake from Qt 5, not Qt 4!  You can 
check which version of qmake is on your path, if any, with the command:

    qmake --version

In addition, if you are using Mac OS X, in order to ensure that qmake can find where boost is installed, 
first add the following lines to the bottom of the file RIVET.pro, changing the paths in the last three lines, 
if necessary, to match the location and version of your copy of Boost.  

    CONFIG += c++11
    QMAKE_CFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9
    QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9

    LIBS += -L"/usr/local/Cellar/boost/1.60.0_2/lib"
    INCLUDEPATH += "/usr/local/Cellar/boost/1.60.0_2/include"

    LIBS += -L"/usr/local/Cellar/boost/1.60.0_2/lib" -lboost_random


After <a href="https://help.github.com/articles/cloning-a-repository/" target="_blank">cloning</a> to $RIVET_DIR:


    cd $RIVET_DIR
    mkdir build
    cd build
    cmake ..
    make
    cd .. 
    qmake
    make
    

You may see compiler warnings during either of the `make` executions. 
These can safely be ignored. 

After this, you will have two executables built: the viewer (RIVET.app, 
on a Mac, RIVET elsewhere), and the computation engine (rivet_console).

It is then necessary to move or symlink the console into the same folder
where the viewer was built. For example, on Mac OS X:

    cd RIVET.app/Contents/MacOS
    ln -s ../../../build/rivet_console
    
On Ubuntu and most other systems:

    ln -s build/rivet_console
    
In the future, all these steps will be automated so that a single cmake
build will create both executables, and put everything in the right place.
     
## Contributing
    
We welcome your contribution! Code, documentation, unit tests, 
interesting sample data files are all welcome!

Before submitting your branch for review, please run the following from the
top level RIVET folder you cloned from Github:

```
clang-format -i **/*.cpp **/*.h
```

This will format the source code using the project's established source
code standards (these are captured in the `.clang-format` file in the
project root directory).
