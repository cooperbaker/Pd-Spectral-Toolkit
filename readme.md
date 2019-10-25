# Pd Spectral Toolkit

The Pd Spectral Toolkit facilitates frequency domain signal processing with a family of spectral transformation, calculation, and data manipulation objects. These objects implement low-level algorithms that simplify patches by handling the complex math and data manipulation commonly required for spectral signal processing.

The toolkit was created by Cooper Baker in 2013, with generous support from a University of California San Diego research grant supervised by Tom Erbe. In 2019 the project was updated as a cross-platform library and added to Pd's external object management system. Special thanks to Miller Puckette for invaluable insight regarding spectral math and the Pd api.

An online manual, code browser, and more detailed information is available here:
### [Pd Spectral Toolkit Webpage](http://www.cooperbaker.com/pd-spectral-toolkit)
<br>

## Manual Installation

- Clone or download this repository
- Copy the _Pd Spectral Toolkit_ folder into your Pd installation
  - i.e. /Library/Pd/Pd Spectral Toolkit in Mac OS
- Edit Pd startup settings to load _pd_spectral_toolkit_
<br>

## How To Build

### All Platforms

- Clone or download this repository
- Clone a fresh copy of Pure Data into the Pd-Spectral-Toolkit folder

        $ git clone https://github.com/pure-data/pure-data.git

- You should now have a folder named _pure-data_ containing the source code of Pd, in the root level of _Pd-Spectral-Toolkit_

### Linux

- Build the toolkit

        $ make linux

### Windows

- Edit the makefile to verify the windows compiler path

        WINDOWSVC = "C:\Program Files . . .

- Build the toolkit

        $ make windows

### Mac OS

- Objects must be compiled with actual gnu gcc, not xcode clang gcc, or else some objects will crash


- Install the [Homebrew](https://brew.sh/) package manager

        $ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

- Install gcc from Homebrew :

        $ brew install gcc

- Build the toolkit

        $ make macos
