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
      positionChildren(element, xPos, 0, n, e);
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
		{"activity", "reporting"}
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
		{"activity", "activated"}
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
		{"activity", "enabled"}
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
      {"symbol", std::string(1, (char)symbol) },
      {"nodes", changedNodes},
      {"rep_nodes", reportNodes }
    };
    out << nlohmann::json::parse(json_object.dump()).dump(4) << std::endl;

    return out.str();

  }
 private:
  static int positionChildren (Element *parent, 
			       int &xPos, 
			       int y, 
			       std::vector<json11::Json> &n, 
			       std::vector<json11::Json> &e) {
    
    if (DEBUG) 
      std::cout << "Running positionChildren on '" << parent->getId() << "' with xPos=" << std::to_string(xPos) << std::endl;

    // Check if element has been visited - prevent logic loops
    if (parent->isMarked())
      return xPos;
    parent->mark();

    // Set type based on element type
    std::string type = "";
    std::string data_string = "{";
    std::string err;
    std::string symbolset = "";
    std::string newSS = "";
    json11::Json data;
    int start_pos = 0;
    switch (parent->getType()) {
    case ElementType::STE_T:
      // Add symbol set to data
      symbolset = static_cast<STE *>(parent)->getSymbolSet();
      newSS = "";
      // Escape all quotes and slashes in symbol set for proper parsing and display
      start_pos = 0;
      while (symbolset[start_pos]) {
	if (symbolset[start_pos] == '"' || symbolset[start_pos] == '\\')
	  newSS += "\\";
	newSS += symbolset[start_pos++];
      }

      data_string += "\"ss\": \"" + newSS + "\", ";

      if (parent->isReporting()) {
	type = "report";
	data_string += "\"rep_code\": \"Report Code: " + parent->getReportCode() + "\", ";
      }
      else if (static_cast<STE *>(parent)->isStart()) {
	type = "start";
	data_string += "\"start\": \"Start Type: " + static_cast<STE *>(parent)->getStringStart() + "\", ";
      }
      else
	type = "node";
     
      break;
    case ElementType::OR_T:
      type = "OR";
      break;
    case ElementType::AND_T:
      type = "AND";
      break;
    case ElementType::COUNTER_T:
      type = "COUNTER";
      break;
    case ElementType::INVERTER_T:
      type = "INVERTER";
      break;
    case ElementType::NOR_T:
      type = "NOR";
      break;
    default:
      type = "def";
    }
    // All elements will have the type parameter at a minimum
    data_string += "\"type\": \"" + type + "\"}";

    data = json11::Json::parse(data_string, err);

    // Base case: no children
    if (parent->getOutputs().empty()) {
      if (DEBUG) 
	std::cout << "*** (Base Case) Adding STE '" << parent->getId() << "' with xPos=" << std::to_string(xPos) << std::endl;
      // Add this node with all current info
      n.push_back(json11::Json::object{
	  {"id", parent->getId() },
	    {"data", data },
	      {"x", std::to_string(xPos) },
		{"y", std::to_string(y) }
	});
      return xPos;
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

	// Only run recursive call if not a self-loop
	int positionOfChild = xPos;
	if (child->getId() != parent->getId())
	  positionOfChild = positionChildren(child, xPos, y+30, n, e);
   
	e.push_back(json11::Json::object {
	    {"id", "e"+std::to_string(e.size()) },
	      {"source", parent->getId() },
		{"target", child->getId() }
	  });

        // low = xPos of first element
	if (&item == &outputs.front()) {
	  low = positionOfChild;
	  if (DEBUG) 
	    std::cout << "'low' of '" << parent->getId() << " after child loop on '" << child->getId() << "' = " << std::to_string(low) << std::endl;
	}
	// high = xPos of last element
	if (&item == &outputs.back()) {
	  high = positionOfChild;
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
	    {"data", data },
	      {"x", std::to_string((low + high + 1)/2) },
		{"y", std::to_string(y) }
	});

      if (DEBUG) 
	std::cout << "*** Adding STE '" << parent->getId() << "' with xPos=" << std::to_string((low+high+1)/2) << std::endl;

      return (low + high + 1)/2;
    }

  }

 private:
  static const bool DEBUG = true;
  
};
