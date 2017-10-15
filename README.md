# RIVET

Program to visualize two-parameter persistent homology. 
Designed by Michael Lesnick and Matthew Wright. 
Created December 2013.  

## Contributors
Madkour Abdel-Rahman (St. Olaf)
Bryn Keller (Intel Labs)
Michael Lesnick (Princeton)
Phil Nadolny (St. Olaf)
William Wang (UPenn)
Matthew Wright (St. Olaf)
Alex Yu (Princeton)
Roy Zhao (UC Berkeley)

## Requirements
Before starting to build RIVET, you will need to have the following installed:
 
* A C++ compiler (g++ or clang are what we use)
* CMake
* Qt 5
* Boost (including boost serialization; version 1.60 or newer required)

Below we give step-by-step instructions for installing these required dependencies and building RIVET on Ubuntu and Mac OS X.  Building RIVET on Windows is not yet supported (we are working on this), but it is possible to build RIVET using the Bash shell on Windows 10.

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
Our experience has been that if Homebrew is installed before XCode, then running qmake during the build process returns an error:

    Project ERROR: Could not resolve SDK Path for 'macosx'
    
To solve the problem, try running:      

    sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer

## Building in the Bash Shell on Windows 10

First, ensure that you have the [Windows 10 Creators Update](https://support.microsoft.com/en-us/instantanswers/d4efb316-79f0-1aa1-9ef3-dcada78f3fa0/get-the-windows-10-creators-update). Then activate the [Windows 10 Bash Shell](https://www.howtogeek.com/249966/how-to-install-and-use-the-linux-bash-shell-on-windows-10/). This will provide a Bash shell with Ubuntu 16.04 inside of Windows 10.

Open the Bash shell and install dependencies. Use the following command to install cmake, a compiler, and Qt5:

    sudo apt-get install cmake build-essential qt5-default qt5-qmake qtbase5-dev-tools 

The Ubuntu 16.04 repositories include Boost 1.58, but RIVET requires Boost 1.60 or newer. The following instructions install Boost 1.64 into `/usr/local/boost_1_64_0/`:

1. Download a compressed Boost file into your home directory:

        cd ~
        wget "https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.bz2"

2. Unpack Boost to `/usr/local/`:

        cd /usr/local
        sudo tar xvjf ~/boost_1_64_0.tar.bz2
        cd boost_1_64_0

3. Setup Boost:

        sudo ./boostrap.sh --prefix=/usr/local
        sudo ./b2
        sudo ./b2 install

4. Return to home directory and delete the compressed Boost file.

        cd ~
        rm boost_1_64_0.tar.bz2

In order to use the RIVET viewer, you must install an X server such as [Xming](https://sourceforge.net/projects/xming/).

It is also necessary to set two environment variables, as follows:

    export LD_LIBRARY_PATH=/usr/local/boost_1_64_0/stage/lib/:$LD_LIBRARY_PATH
    export DISPLAY=:0

These environment variables will be reset when you close the Bash shell. To avoid having to run the two lines above when you reopen the shell, add these lines to the end of the file `~/.bashrc`.

You are now ready to build RIVET. Follow the instructions in the section of this readme under the heading [Building on Ubuntu: Building RIVET](https://github.com/rivetTDA/rivet/#building-rivet).


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

## Acknowledgements

This material is based upon work supported by the National Science Foundation under Grant Number 1606967. Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.

Additional support has been been provided by the Institute for Mathematics and its Applications, Columbia University, Princeton University, St. Olaf College, and the NIH (grant U54-CA193313-01).
