# ANMLViewer Changelog

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
