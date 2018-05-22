#include <json11.hpp>
#include <json.hpp>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <math.h>
#include "automata.h"
#include "element.h"
#include "ste.h"
#include <iostream>

class ForceDirectedLayout {
  
 private:
  static const bool DEBUG = true;

  static void randomPosition(Automata *a, std::unordered_map<std::string, std::pair<double, double>> &positions) {
    
    std::cout << "Randomly placing all nodes...";
    int dim = 100;
    auto elements = a->getElements();
    for (auto e:elements) {
      float x = rand() % dim;
      float y = rand() % dim;
      positions[e.second->getId()] = std::make_pair(x, y);
    }
    std::cout << "done." << std::endl;
  }

  static double squareDistance(std::pair<double,double> p1, std::pair<double,double> p2) {
    return pow(p1.first - p2.first, 2) + pow(p1.second - p2.second, 2);
  }

  static void calculateSpringForce(Element *parent,
				   double spring_constant,
				   std::unordered_map<std::string, std::pair<double, double>> &positions,
				   std::unordered_map<std::string, std::pair<double, double>> &vectors) {
    if (parent->isMarked())
      return;
    parent->mark();
    
    std::string id = parent->getId();
    for (auto& item : parent->getOutputSTEPointers()) {
      Element *child = item.first;
      std::string id2 = child->getId();
      double forceMag = -spring_constant / sqrt(squareDistance(positions[id], positions[id2]));
      double angle = atan2(positions[id2].second - positions[id].second, positions[id2].first - positions[id].first);
	
      vectors[id].first += forceMag * cos(angle);
      vectors[id].second += forceMag * sin(angle);
      vectors[id2].first -= forceMag * cos(angle);
      vectors[id2].second -= forceMag * sin(angle);

      calculateSpringForce(child, spring_constant, positions, vectors);
    }
  }

  static void addJson(Element *parent,
		      std::unordered_map<std::string, std::pair<double, double>> &positions,
		      std::vector<json11::Json> &n,
		      std::vector<json11::Json> &e) {

    if (parent->isMarked())
      return;
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

    n.push_back(json11::Json::object{
	{"id", parent->getId() },
	  {"data", data },
	    {"x", positions[parent->getId()].first },
	      {"y", positions[parent->getId()].second }
      });

    for (auto item:parent->getOutputSTEPointers()) {
      Element *child = item.first;
      e.push_back(json11::Json::object {
	  {"id", "e"+std::to_string(e.size()) },
	    {"source", parent->getId() },
	      {"target", child->getId() }
	});
      addJson(child, positions, n, e);
    }

  }

 public:

  static std::string writeToJSON(Automata *a) {
    std::cout << "Beginning placement process..." << std::endl;
    
    std::unordered_map<std::string, std::pair<double, double>> positions;
    std::unordered_map<std::string, std::pair<double, double>> vectors;
    std::vector<json11::Json> n;
    std::vector<json11::Json> e;

    // Step one: Randomly position all nodes
    std::cout << "Step one: randomly position all nodes" << std::endl;
    randomPosition(a, positions);
    // Step two: vectors
    // Step three: loop through all elements and calulcate force vector from other STEs
    auto elements = a->getElements();
    double k = 10;
    for (int i = 0; i < 500; i++ ) {
      std::cout << "Caluclating repulsive force vectors..." << std::endl;
      for (auto item1:elements) {
	Element *e1 = item1.second;
	int degree1 = e1->getOutputs().size();
	e1->unmark();
	std::string id = e1->getId();
	vectors[id] = std::make_pair(0.0,0.0);
	for (auto item2:elements) {
	  Element *e2 = item2.second;
	  int degree2 = e2->getOutputs().size();
	  std::string id2 = e2->getId();
	  if (id != id2) {
	    // F = k * (deg1 + 1) * (deg2 + 1) / d^2
	    double forceMag = k * (degree1 + 1) * (degree2 + 1) / squareDistance(positions[id], positions[id2]);
	    double angle = atan2(positions[id2].second - positions[id].second, positions[id2].first - positions[id].first);
	    vectors[id].first += forceMag * cos(angle);
	    vectors[id].second += forceMag * sin(angle);
	  }
	}
	std::cout << "\tRepulsive force vector = <" << vectors[id].first << "," << vectors[id].second << ">" << std::endl;
      }
      std::cout << "done." << std::endl;

      // Step four: calculate spring forces recursively
      std::cout << "Caluclating spring force vectors..." << std::endl;
      auto starts = a->getStarts();
      for (auto s:starts) {
	calculateSpringForce(s, k, positions, vectors);
      }
      std::cout << "done." << std::endl;


      // Step five: update position of all nodes
      std::cout << "Updating positions of nodes...";
      for (auto e:elements) {
	std::string id = e.second->getId();
	positions[id].first += vectors[id].first;
	positions[id].second += vectors[id].second;
	std::cout << "\tVector = <" << vectors[id].first << "," << vectors[id].second << ">" << std::endl;
      }
      std::cout << "done." << std::endl;
      // Loop x times
      std::cout << "Loop number " << (i+1) << " complete.\n" << std::endl;
    }
    std::cout << "\n*** Completed entire loop ***" << std::endl;

    // Step six: JSONify everything
    for (auto e:elements)
      e.second->unmark();
    std::cout << "Converting graph to JSON..." << std::endl;
    for (auto s:a->getStarts()) {
      addJson(s, positions, n, e);
    }
    std::cout << "done." << std::endl;
    
    // Creating and outputting JSON object as string
    std::stringstream out;
    json11::Json json_object = json11::Json::object {
      {"nodes", n},
      {"edges", e}
    };
    out << nlohmann::json::parse(json_object.dump()).dump(4) << std::endl;
    return out.str();
  }
};
