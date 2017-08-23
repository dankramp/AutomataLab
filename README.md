# Automata Lab
Visualization web client for VASim by Dan Kramp

This software allows the user to visualize and simulate large-scale automata. It is highly interactive and much faster than existing automata displaying software.

![AutomataLab Fullscreen Demo](http://i.imgur.com/alfpobM.png?1)

## How To Install + Run
This tool uses [Wt, a C++ Web Toolkit](https://www.webtoolkit.eu/wt) v.3.7.7 for deployment and HTML generation. This must be installed on your computer before you can deploy locally (see below for installation).
It also utilizes [Sigma.js](https://github.com/jacomyal/sigma.js) for graph display/interaction and [VASim](https://github.com/jackwadden/VASim/) for automata simulation. Both git directories are included.

First, clone the repo:
```
git clone https://github.com/dankramp/AutomataLab.git
cd AutomataLab
```

Once all the files are downloaded, run the following (downloads and installs required dependencies, compiles, and runs):
```
sudo apt-get install witty witty-dev g++-5 nasm libboost-all-dev
make run
```
Then in your browser, navigate to the link provided by Wt:
```
    [info] "wthttp: started server: http://0.0.0.0:9090"
```

## How To Use

See the [Wiki page](https://github.com/dankramp/AutomataLab/wiki) for complete details on using and coding Automata Lab.
