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

Below we give step-by-step instructions for installing these required dependencies and building RIVET on Ubuntu and Mac OS X.  Currently, use of RIVET in Windows is not supported; we are working on this.

## Building On Ubuntu

### Installing Dependencies
To install dependencies on Ubuntu, we suggest that you first upgrade to Ubuntu 16.10; the Ubuntu 16.04 package manager only installs Boost 1.58, whereas RIVET requires Boost version 1.60 or higher.

On Ubuntu 16.10, installation of dependencies should be relatively simple:

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
    
If a popup window appears, click the "Install" button, and accept the license agreement.  

Next, install XCode from the App Store, if you've not done so already.  You will also need accept the license agreement for XCode, which you can do from the command line by running:

    sudo xcodebuild -license

To install the remaining packages, we recommend using the package manager [Homebrew](http://brew.sh/).  To install Homebrew:

    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"    
    
Now install cmake, qt5, and boost:
    
    brew install cmake qt5 boost
    
Please note that, as of the time of writing, brew installs `qmake` in a version-specific folder under 
/usr/local/Cellar/qt5/[my_version_#]/bin, and does not add it to your `PATH`. You can find
the folder where qt5 is installed using this command:

    brew info qt5 | grep Cellar | cut -d' ' -f1

In fact, let's store that in a variable so we can use it below:
    
    export QT_BASE=`brew info qt5 | grep Cellar | cut -d' ' -f1`

Finally, in order to ensure that qmake can find where boost is installed, add the following lines to the bottom of the file RIVET.pro, changing the paths in the last three lines, if necessary, to match the location and version of your copy of Boost.  

    CONFIG += c++11
    QMAKE_CFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9
    QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9

    LIBS += -L"/usr/local/Cellar/boost/1.63.0/lib"
    INCLUDEPATH += "/usr/local/Cellar/boost/1.63.0/include"

    LIBS += -L"/usr/local/Cellar/boost/1.63.0/lib" -lboost_random

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

### Troubleshooting
Our experience has been that if Homebrew is installed before XCode, then running qmake during the build process returns an error

    Project ERROR: Could not resolve SDK Path for 'macosx'
    
To solve the problem, try running      

    sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer

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
