# ANMLViewer Changelog
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
