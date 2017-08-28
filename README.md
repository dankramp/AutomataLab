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

Once all the files are downloaded, run the following (downloads and installs required dependencies):
```
sudo apt-get install witty witty-dev g++-5 nasm libboost-all-dev
```
To compile and run each time after, use `make run`. To skip compiling, simply use `./run.sh` instead. Then in your browser, navigate to the link provided by Wt:
```
    [info] "wthttp: started server: http://0.0.0.0:9090"
```

## How To Use

See the [Wiki page](https://github.com/dankramp/AutomataLab/wiki) for complete details on using and coding Automata Lab.

## Citing Automata Lab
If you use Automata Lab as a part of your work, please use the following citation:

Kramp, D., Wadden, J., and Skadron, K. "Automata Lab: An Open-Source Automata Visualization, Simulation, and Manipulation Tool." University of Virginia, Tech Report #CS2017-03, 2016.

```
@techreport{vasim,
    Author = {Kramp, Dan and Wadden, Jack and Skadron, Kevin},
    Institution = {University of Virginia},
    Number = {CS2017-03},
    Title = {{Automata Lab: An Open-Source Automata Visualization, Simulation, and Manipulation Tool}},
    Year = {2017}}
```

