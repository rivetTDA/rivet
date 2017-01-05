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
Before starting to build RIVET, you will need to have the following installed:
 
* A C++ compiler (g++ or clang are what we use)
* CMake
* Qt 5
* Boost (including boost serialization; version 1.60 or newer required)

*NOTE* that currently CMake does not recognize Boost 1.63 or newer 
(see https://gitlab.kitware.com/cmake/cmake/merge_requests/361), 
please use Boost 1.60-1.62 to avoid this problem.

Below we give step-by-step instructions for installing these required dependencies and building RIVET on Ubuntu and Mac OS X.  
TODO: Add instructions for Windows.  

## Building On Ubuntu

### Installing Dependencies
On Ubuntu, installation of dependencies should be relatively simple:

    sudo apt-get install cmake qt5-default qt5-qmake qtbase5-dev-tools libboost-all-dev

### Building RIVET 

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

After this, you will have two executables built: the viewer (RIVET), 
and the computation engine (rivet_console).

It is then necessary to move or symlink the console into the same folder
where the viewer was built. On Ubuntu and most other systems:

    ln -s build/rivet_console
    
In the future, all these steps will be automated so that a single cmake build will create both executables, and put everything in the right place.    

## Building On Mac OS X

### Installing Dependencies

First,  ensure you have the XCode Command Line Tools installed by running:

    # only needed if you've never run it before, (running it again doesn't hurt anything)
    # installs XCode Command Line Tools
    xcode-select --install

Next, install XCode from the App Store, if you've not done so already.

TODO: Does this process require the user to accept a license agreement?  (I suspect so.)  Should this be mentioned?

To configure XCode to run as required by qt5, enter

    sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer

TODO: Is the above necessary?  I think so but, I'm not sure.  It was for me, but I proceeded in a different order, installing Brew first.

To install the remaining packages, we recommend using Homebrew, which must be 
installed separately using the instructions at the [Homebrew web site](http://brew.sh/).
    
    # now install the needed packages 
    brew install cmake qt5
    
Since boost 1.63 (which is currently the version that brew installs) or greater 
isn't recognized by CMake 
[details here again](https://gitlab.kitware.com/cmake/cmake/merge_requests/361),
we need to select a specific version for Mac:
    
    # Make older versions available to homebrew
    brew tap homebrew/versions
    
    # Install a CMake-findable version of Boost
    brew install boost160 
    
Please note that, as of the time of writing, brew installs `qmake` in a version-specific folder under 
/usr/local/Cellar/qt5/[my_version_#]/bin, and does not add it to your `PATH`. You can find
the folder where qt5 is installed using this command:

    brew info qt5 | grep Cellar | cut -d' ' -f1

In fact, let's store that in a variable so we can use it below:
    
    export QT_BASE=`brew info qt5 | grep Cellar | cut -d' ' -f1`

Finally, in order to ensure that qmake can find where boost is installed, add the following lines to the bottom of the file RIVET.pro, changing the paths in the last three lines, 
if necessary, to match the location and version of your copy of Boost.  

    CONFIG += c++11
    QMAKE_CFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9
    QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9

    LIBS += -L"/usr/local/Cellar/boost/1.60.0_2/lib"
    INCLUDEPATH += "/usr/local/Cellar/boost/1.60.0_2/include"

    LIBS += -L"/usr/local/Cellar/boost/1.60.0_2/lib" -lboost_random

### Building RIVET
After <a href="https://help.github.com/articles/cloning-a-repository/" target="_blank">cloning</a> to $RIVET_DIR:

    cd $RIVET_DIR
    mkdir build
    cd build
    cmake ..
    make
    cd .. 
    $QT_BASE/bin/qmake
    make    
    
You may see compiler warnings during either of the `make` executions.
These can safely be ignored. 

After this, you will have two executables built: the viewer (RIVET.app),
and the computation engine (rivet_console).
   
It is then necessary to move or symlink the console into the same folder where the viewer was built:

    cd RIVET.app/Contents/MacOS
    ln -s ../../../build/rivet_console   

In the future, all these steps will be automated so that a single cmake build will create both executables, and put everything in the right place.

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
