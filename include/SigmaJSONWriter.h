#include <json11.hpp>
#include <json.hpp>
#include <vector>
#include "automata.h"
#include "element.h"
#include "ste.h"
#include <iostream>

class SigmaJSONWriter {
 public:

  static std::string writeToJSON(Automata *a) {

    // Wash all dishes before you eat
    for (auto e : a->getElements())
      e.second->unmark();

    // xPos is first possible x position for placement of nodes
    int xPos = 0;
    // nodes
    std::vector<json11::Json> n;
    // edges
    std::vector<json11::Json> e;

    // Loop through all starting elements
    for (auto element : a->getStarts()) {
      xPos = positionChildren(element, xPos, 0, n, e);
      xPos++;
    }
    
    // Wash dishes after eating, too
    for (auto e : a->getElements())
      e.second->unmark();
    
    // Creating and outputting JSON object as string
    std::stringstream out;
    json11::Json json_object = json11::Json::object {
	{"nodes", n},
	  {"edges", e}
    };
    out << nlohmann::json::parse(json_object.dump()).dump(4) << std::endl;
    
    return out.str();
    
  }
  static std::string simulateStep(Automata *a, uint8_t symbol) {

    // JSON output vectors
    std::vector<json11::Json> changedNodes;
    std::vector<json11::Json> reportNodes;

    a->simulate(symbol);
    auto enabledCount = a->getEnabledCount();

    // *** ADD CHANGED STES TO JSON GRAPH ***
    std::queue <STE *> tmp;
    std::queue <Element *> activatedSTEs = a->getActivatedLastCycle();
    std::queue <Element *> enabledSTEs = a->getEnabledLastCycle();
    std::queue <Element *> reportingSTEs = a->getReportedLastCycle();

    // Add all reporting elements first
    while (!reportingSTEs.empty()) {
      STE * s = static_cast<STE *>(reportingSTEs.front());
      tmp.push(s);
      reportingSTEs.pop();
      // Always avoid duplicates
      if (!s->isMarked()) { 
	changedNodes.push_back(json11::Json::object { 
	    {"id", s->getId() },
	      {"count", std::to_string(enabledCount[s]) },	      
		{"color", "rgb(255,0,255)"}
	  });
	reportNodes.push_back(json11::Json::object { 
	    {"id", s->getId() },
	      {"rep_code", s->getReportCode() }
	  });
      }
      s->mark();
    }
    // Reload to unmark later
    while (!tmp.empty()) {
      reportingSTEs.push(tmp.front());
      tmp.pop();
    }
    
    // Add activated elements next
    while (!activatedSTEs.empty()) {
      STE * s = static_cast<STE *>(activatedSTEs.front());
      tmp.push(s);
      activatedSTEs.pop();
      // Avoid duplicates from reporting
      if (!s->isMarked())
	changedNodes.push_back(json11::Json::object { 
	    {"id", s->getId() },
	      {"count", std::to_string(enabledCount[s]) },
		{"color", "rgb(0,255,0)"}
	  });
      s->mark();
    }
    // Reload to unmark later
    while (!tmp.empty()) {
      activatedSTEs.push(tmp.front());
      tmp.pop();
    }
    
    // Finally, add all enabled elements
    while (!enabledSTEs.empty()) {
      STE * s = static_cast<STE *>(enabledSTEs.front());
      tmp.push(s);
      enabledSTEs.pop();
      // Avoid duplicates from activated
      if (!s->isMarked())
	changedNodes.push_back(json11::Json::object { 
	    {"id", s->getId() },
	      {"count", std::to_string(enabledCount[s]) },
		{"color", "rgb(255,255,0)"}
	  });
      s->mark();
    }
    
    // Unmark all enabled, activated & reporting elements
    while (!tmp.empty()) {
      tmp.front()->unmark();
      tmp.pop();
    }
    while (!activatedSTEs.empty()) {
      STE * s = static_cast<STE *>(activatedSTEs.front());
      s->unmark();
      activatedSTEs.pop();
    }
    while (!reportingSTEs.empty()) {
      STE * s = static_cast<STE *>(reportingSTEs.front());
      s->unmark();
      reportingSTEs.pop();
    }
    
    // Output to file
    std::stringstream out;
    json11::Json json_object = json11::Json::object {
      {"nodes", changedNodes},
      {"rep_nodes", reportNodes }
    };
    out << nlohmann::json::parse(json_object.dump()).dump(4) << std::endl;
    
    return out.str();
    
  }
 private:
  static int positionChildren (Element *parent, 
			       int xPos, 
			       int y, 
			       std::vector<json11::Json> &n, 
			       std::vector<json11::Json> &e) {
    
    if (DEBUG) 
      std::cout << "Running positionChildren on '" << parent->getId() << "' with xPos=" << std::to_string(xPos) << std::endl;

    // Check if element has been visited - prevent logic loops
    if (parent->isMarked())
      return xPos;
    parent->mark();

    // Set color and label based on element type
    std::string color = "";
    std::string label = "";
    switch (parent->getType()) {
    case ElementType::STE_T:
      if (parent->isReporting())
	color = "rgb(255,150,0)";
      else if (static_cast<STE *>(parent)->isStart())
	color = "rgb(100,100,100)";
      else
	color = "rgb(158,185,212)";
      //color= "#9eb9d4";
      
      label = static_cast<STE *>(parent)->getSymbolSet();
      break;
    case ElementType::OR_T:
      color = "rgb(255,0,255)";
      label = "OR";
      break;
    case ElementType::AND_T:
      color = "rgb(255,0,255)";
      label = "AND";
      break;
    case ElementType::COUNTER_T:
      color = "rgb(255,0,255)";
      label = "COUNTER";
      break;
    case ElementType::INVERTER_T:
      color = "rgb(255,0,255)";
      label = "INVERTER";
      break;
    case ElementType::NOR_T:
      color = "rgb(255,0,255)";
      label = "NOR";
      break;
    }
      
    // Base case: no children
    if (parent->getOutputs().empty()) {
      if (DEBUG) 
	std::cout << "*** (Base Case) Adding STE '" << parent->getId() << "' with xPos=" << std::to_string(xPos) << std::endl;
      // Add this node with all current info
      n.push_back(json11::Json::object{
	{"id", parent->getId() },
	  {"label", label },
	    {"color", color },
	      {"x", std::to_string(xPos) },
		{"y", std::to_string(y) }, 
		  {"type", "fast"}
      });
    }

    // Recursive case
    else {

      // x position of n is (low + high + 1) / 2
      // low is x position of leftmost child, high is of rightmost child
      
      int low = xPos, high = xPos;
      std::vector<std::pair<Element *, std::string>> outputs = parent->getOutputSTEPointers();
      for (auto& item : outputs ) {
	Element *child = item.first;
	
	if (DEBUG) 
	  std::cout << "Running loop on '" << parent->getId() << "'s child '" << child->getId() << "' with xPos=" << std::to_string(xPos) << std::endl;
	
	std::string type = "arrow";
	// Self-loop handling
	if (child->getId() == parent->getId())
	  type = "curveArrow";
	// Only run recursive call if not a self-loop
	else
	  xPos = positionChildren(child, xPos, y+10, n, e);
	
	e.push_back(json11::Json::object {
	    {"id", "e"+std::to_string(e.size()) },
	      {"source", parent->getId() },
		{"target", child->getId() },
		  {"type", type}
	  });

        // low = xPos of first element
	if (&item == &outputs.front()) {
	  low = xPos;
	  if (DEBUG) 
	    std::cout << "'low' of '" << parent->getId() << " after child loop on '" << child->getId() << "' = " << std::to_string(low) << std::endl;
	}
	// high = xPos of last element
	if (&item == &outputs.back()) {
	  high = xPos;
	  if (DEBUG) 
	    std::cout << "'high' of '" << parent->getId() << " after child loop on '" << child->getId() << "' = " << std::to_string(high) << std::endl;
	}

	// Another self-loop handler -- does not move xPos if child == parent
	// This can't be added earlier or else it interferes with 'high' and 'low'
	if (child->getId() != parent->getId())
	  xPos++;

	if (DEBUG) 
	  std::cout << "xPos is now " << std::to_string(xPos) << " after child loop on '" << child->getId() << "'" << std::endl;
      }
      xPos--;

      
    n.push_back(json11::Json::object{
	{"id", parent->getId() },
	  {"label", label },
	    {"color", color },
	      {"x", std::to_string((low + high + 1)/2) },
		{"y", std::to_string(y) },
		  {"type", "fast"}
      });

    if (DEBUG) 
      std::cout << "*** Adding STE '" << parent->getId() << "' with xPos=" << std::to_string((low+high+1)/2) << std::endl;

    }

    return xPos;
  }

 private:
  static const bool DEBUG = false;
  
};
