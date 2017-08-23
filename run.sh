#!/bin/bash

LD_LIBRARY_PATH=/VASim/MNRL/C++:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

./VASimVis --docroot . --http-address 0.0.0.0 --http-port 9090 --config wt_config.xml
