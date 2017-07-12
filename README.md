# ANMLViewer
Visualization web client for VASim by Dan Kramp

This software allows the user to visualize and simulate large-scale automata. It is highly interactive and much faster than existing automata displaying software.

![ANMLViewer Fullscreen Demo](https://i.imgur.com/cxZNPZf.png)

## How To Install + Run
This tool uses [Wt, a C++ Web Toolkit](https://www.webtoolkit.eu/wt) v.3.7.7 for deployment and HTML generation. This must be installed on your computer before you can deploy locally.
It also utilizes [Sigma.js](https://github.com/jacomyal/sigma.js) for graph display/interaction and [VASim](https://github.com/jackwadden/VASim/) for automata simulation. Both git directories are included.

Once all the files are downloaded, run the following:
```
    user:~/WebViewer$ make
    user:~/WebViewer$ run.sh
```
Then in your browser, navigate to the link provided by Wt:
```
    [info] "wthttp: started server: http://0.0.0.0:9090"
```

## How To Use
1. Select a local automata file (.anml/.mnrl) or a preset ANMLZoo file from the drop down.
2. Choose which optimizations to apply to the automata and click "Generate".
3. The automata is displayed and the user may interact with it in a number of ways:
    - Zoom in/out using scroll wheel or touch-screen methods
    - Drag screen to reposition graph
    - Change the node display size, arrow size, edge thickness, and graph width in the "Graph Settings" tab at the bottom
    - Toggle "Render Edges" setting for faster render times and manipulation of the view
    - Toggle "Heat Map Mode" to see the nodes colored based on number of times the node has been enabled
    - Hover over a node to see its symbol set, or click to see all connected nodes
    - Click "Reset Camera" to reset view to initial zoom and position
    - The header at the top can be hidden/shown at any time by clicking the bar along its bottom edge
4. Select a local text input file to simulate on (if an ANMLZoo file was chosen, the text file will have already been loaded but can be overwritten by uploading another).
5. The text file is displayed as a character stream along the bottom of the simulation window, displaying up to the first 1000 characters.
6. Once both the automata file and text file are loaded, click "Simulate >>" in the header to begin simulating the automata.
    - The character that is currently being simulated on is highlighted yellow in the character stream
7. After the simulation cache is generated, the user may simulate the automata in a number of ways:
    - Step backward/forward one character in the stream using the "<< Step" and "Step >>" respectively
    - The user may step backwards until the beginning of the cache is reached; forward simulation can occur until the end of the input stream
    - Play the simulation forward by pressing "Play Simulation"
    - Toggle displaying the character stream as hex values using the "Hex Char Mode" check box
    - View the ID and report code of all reporting STEs on any given character by hovering over characters that are highlighted purple
8. Selecting a new automata or input file will reset the simulation and allow the user to restart without refreshing the page.
