#!/bin/bash

# Find the root of AutomataLab
AUTOMATA_LAB_ROOT=$(dirname "$(readlink -f "$0")")
export LD_LIBRARY_PATH=$AUTOMATA_LAB_ROOT/VASim/MNRL/C++:${LD_LIBRARY_PATH}

$AUTOMATA_LAB_ROOT/VASimVis --docroot . --http-address 0.0.0.0 --http-port 9090 --config $AUTOMATA_LAB_ROOT/wt_config.xml
