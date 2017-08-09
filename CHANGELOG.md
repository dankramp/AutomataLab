# ANMLViewer Changelog
### 1.2 - release (8/3/17)
- __Stage Changes__
  - Added context menus:
    - Right-clicking the stage launches context menu at that point with several options:
      - 'Search by ID' allows the user to serach for a specific STE by its ID
      - 'Show All Nodes' reveals every node that may have been hidden
      - 'Toggle Editor Mode' switches to/from Editor Mode (see below for info)
      - 'Reset Camera' doubles the functionality of the normal 'Reset Camera' button (both now reset rotation as well)
    - Right-clicking a node displays some info and options:
      - Shows an STE's Id, symbol set, start type (if applicable) and report code (if applicable)
      - 'Show/Hide Children' will toggle the visibility of all descendants of the clicked node
      - 'Solo Children' will hide all nodes on graph that are not descendants of the clicked node
      - '- Print Node Data -' outputs the node JSON data to the browser's console (useful for debugging)
  - Added 'View Graph Legend' button to 'Graph Settings' tab
    - Displays small popup window with color representations of different states of STEs
    - Automatically updates colors based on global settings file (see below about global settings)
- __Added "Editor Mode"__
  - Can be switched in/out of by right-clicking the stage and selecting 'Toggle Editor Mode'
  - Allows the user to move, create, delete, or change STEs and their connections
    - STEs can be moved around by clicking and dragging
    - Multiple STEs can be selected at once in a number of ways:
      - 'Lasso Tool' allows the user to draw a shape around all nodes they wish to select
      - 'Box Select' selects all nodes within a rectangular region drawn by the user
      - Pressing the spacebar then 'A' selects all nodes
      - Pressing the spacebar then 'U' unselects all nodes
      - Pressing the spacebar then 'E' selects the immediate neighbors of all selected nodes
      - Pressing the spacebar then left clicking selects the clicked node in addition
  - Right-clicking the stage provides a number of options:
    - 'Add STE' shows a popup that allows the user to create a new STE that is added to the automata where the user clicks
    - 'Show All Nodes' displays all previously hidden nodes
    - 'Toggle Editor Mode' exits the editor
    - 'Reset Camera' resets the camera view
    - '- Print All Node Data -' outputs all node JSON data to the browser's console (useful for debugging)
  - Right-clicking an STE also provides a number of options:
    - First displays the same data as shown in the normal mode
    - 'Add Outgoing Connection' adds a connection from the current STE to the next clicked one
      - Clicking the current STE creates a self-loop
      - Clicking the stage will not add a connection
    - 'Change Data' provides popup similar to creation popup and allows user to change any data about an STE
    - 'Delete STE' removes the STE from the automata and all related connections
    - 'Print Node Data' has similar function from normal mode
  - Right-clicking a connection provides a few options:
    - 'Selected Connected STEs' selects the STEs on either ends of the connecting edge
    - 'Remove Edge' deletes the connection from the automata
  - 'Save Automata File' downloads a .anml file containing the data of the current automata displayed
  - 'Save Graph Data' downloads a .json file containing the position data of all nodes and edges
    - This is useful when a preferable layout has been achieved and can be used when reloading automata
- Added 'Positioning Method' to automata constructor settings
  - Allows user to select different positioning method (more can be added in future)
  - Custom positioning allows user to upload JSON graph file that is applied to layout
    - __This file must match all IDs of STEs and connections properly or errors will likely occur__
    - This file can be generated in editor mode by clicking 'Save Graph Data' button after editing
- Added global settings file for visual customization
  - *js/ViewerSettings.js* contains intuitively named properties for editing some default visual settings
  - All changes are reflected after page reload and graph legend also displays updated color scheme

- Smaller changes:
  - Added glyphs to buttons for more intuitive user experience
  - Added rotation slider to 'Graph Settings' to rotate camera
  - URL will automatically update to reflect when user loads an ANMLZoo file with certain optimizations
  - Removed ability to upload JSON file as automata because JSON does not actually construct an automata
  - Properly implemented self-loops in renderers!
  - Catch all VASim errors so the server no longer crashes if problem occurs in constucting automata
  - Changed 'Stop on Report' checkbox to selector
    - 'Stop on cycle' allows user to enter cycle # to stop simulation on

### 1.14 - release (7/19/17)
- Added cycle count to 'Reporting STE' popover on character stream
- Added more metadata to STE label
  - Changed label code to handle displaying multiple lines of info
  - By default, all nodes on will show the STE's id and symbol set (ss)
  - Reporting STEs and start STEs will display report code and start type, respectively
- Changed function on clicking STE to show all descendants instead of immediately connected nodes
- Character stream now shows all reporting steps on cache load instead of when passed
- Changed 'Automata Construction Options' menu:
  - Removed 'Local Optimization'
  - Added optional 'Fan-In/Fan-Out Limit'
  - __This changes the format for GET data__
    - New format is `?a=AutomataName&o=2bitOptCode&fi=fanInLimit&fo=fanOutLimit`
    - `?a=Brill&o=11&fo=2` would load Brill with global optimization, remove OR gates, and have a fan out limit of 2
- Changed 'Graph Settings' tab:
  - Removed 'Arrow Size' slider because WebGL renderer does not display changes in arrow size
  - Changed 'Graph Width' slider to 'Width/Height Ratio' due to Sigma autoscaling behavior
  - Revisited sliders for node size, edge thickness and graph width to scale geometrically with greater ranges

### 1.13 - release (7/14/17)
- Improved interactivity of character stream
  - Greyed out characters are not in the cache and therefore cannot be reached (unless simulating forward)
  - Characters in the cache can be clicked on and the corresponding simulation step will be shown
    - If a user jumps past a reporting step, the character will still turn purple in the character stream
- URL can handle GET data to send pre-optimized ANMLZoo automata in the following format: `?a=AutomataName&o=3bitOptCode`
  - Example: appending `?a=Brill&o=100` will preload Brill with a global optimization (other bits are local opt/remove OR gates)
- Added new features to Simulation Tools:
  - 'Cycle Index' counter displays current cycle count
  - 'Stop on Report' checkbox will stop a simulation that is being played if it reaches a reporting step
  - 'Download Report Record' button will save a file called 'reports.txt' that contains information about all reporting STEs for all previously and currently cached steps
- Added 'Type Input' feature near input upload to manually enter an input stream
- Fixed bugs with uploading new files after starting simulation
	
### 1.12 - release (7/10/17)
- Added "Play Speed" range slider to add delay to between simulation steps
  - 'Fastest' adds no delay; 'Slowest' adds 2 seconds between steps
- Moved all range slider handling to Javascript so it is responsive without server interaction
- Removed unneeded functions from VASimVis.cc

### 1.11 - release (7/10/17)
- Implemented new cache system code
  - Generates 100 graphs immediately
  - Recaches when simulation index is 25 from end of cache
  - Caches 100 new steps, then removes steps from beginning until 25 from index
- __Moved entire cache system to JavaScript__
  - This allows simulation to proceed much faster due to connectionless progression
  - Once cache is generated, simulation manipulation could occur even if server crashed
  - JavaScript sends signal to C++ to generate new cache at specific threshold 
  - Simulation can now run uninterrupted (even while generating new cache) 
	
### 1.1 - release
- First major upload -- see usability features in README.md
