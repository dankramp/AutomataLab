#include <Wt/Http/Client>
#include <Wt/Http/Request>
#include <Wt/WApplication>
#include <Wt/WAnchor>
#include <Wt/WBreak>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WCheckBox>
#include <Wt/WFileUpload>
#include <Wt/WLink>
#include <Wt/WPushButton>
#include <Wt/WSlider>
#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WTableRow>

#include <iostream>
#include <fstream>

#include "automata.h"
#include "SigmaJSONWriter.h"

class VASimViz : public Wt::WApplication
{
public:
  VASimViz(const Wt::WEnvironment& env);
  void newFileUploaded();
  void loadTextFile(std::string fn);
  void handleAutomataFile(bool global, bool local, bool OR, std::string fn);
  void simulateAutomata(int prevIndex, int index, char symbol);
  void loadDemoGraph(int index);

private:
  std::string fn;
  std::string input_string;

  Wt::WText *modal_message = new Wt::WText();
  Wt::WTable *input_display_table;
  Wt::WPushButton *sim_step;
  Wt::WPushButton *simulate;

  Automata ap;

  bool automataUploaded = false;
  bool validAutomata = false;
  bool validInput = false;
  bool playSim = false;
  int simIndex = 0;
};

VASimViz::VASimViz(const Wt::WEnvironment& env)
  : Wt::WApplication(env)
{
  setTitle("ANML Viewer");
  // Style sheets
  Wt::WApplication::instance()->useStyleSheet("https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css");
  Wt::WApplication::instance()->useStyleSheet("styles.css");
  
  // Imports
  Wt::WApplication::instance()->require("/sigma.js/src/sigma.core.js");
  Wt::WApplication::instance()->require("/sigma.js/src/conrad.js");
  Wt::WApplication::instance()->require("/sigma.js/src/utils/sigma.utils.js");
  Wt::WApplication::instance()->require("/sigma.js/src/utils/sigma.polyfills.js");
  Wt::WApplication::instance()->require("/sigma.js/src/sigma.settings.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.dispatcher.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.configurable.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.graph.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.camera.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.quad.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.edgequad.js");
  Wt::WApplication::instance()->require("/sigma.js/src/captors/sigma.captors.mouse.js");
  Wt::WApplication::instance()->require("/sigma.js/src/captors/sigma.captors.touch.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/sigma.renderers.canvas.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/sigma.renderers.webgl.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/sigma.renderers.svg.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/sigma.renderers.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.nodes.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.nodes.fast.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.edges.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.edges.fast.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.edges.arrow.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.labels.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.hovers.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.nodes.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edges.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edges.curve.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edges.arrow.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edges.curvedArrow.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edgehovers.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edgehovers.curve.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edgehovers.arrow.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.edgehovers.curvedArrow.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/canvas/sigma.canvas.extremities.def.js");  
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/svg/sigma.svg.utils.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/svg/sigma.svg.nodes.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/svg/sigma.svg.edges.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/svg/sigma.svg.edges.curve.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/svg/sigma.svg.labels.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/svg/sigma.svg.hovers.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/middlewares/sigma.middlewares.rescale.js");
  Wt::WApplication::instance()->require("/sigma.js/src/middlewares/sigma.middlewares.copy.js");
  Wt::WApplication::instance()->require("/sigma.js/src/misc/sigma.misc.animation.js");
  Wt::WApplication::instance()->require("/sigma.js/src/misc/sigma.misc.bindEvents.js");
  Wt::WApplication::instance()->require("/sigma.js/src/misc/sigma.misc.bindDOMEvents.js");
  Wt::WApplication::instance()->require("/sigma.js/src/misc/sigma.misc.drawHovers.js");
  // Sigma plugins
  Wt::WApplication::instance()->require("/sigma.js/plugins/sigma.parsers.json/sigma.parsers.json.js");
  Wt::WApplication::instance()->require("/sigma.js/plugins/sigma.renderers.customShapes/shape-library.js");
  Wt::WApplication::instance()->require("/sigma.js/plugins/sigma.renderers.customShapes/sigma.renderers.customShapes.js");

  /**
   * The fixed page header
   **/
  Wt::WContainerWidget *fixed_header = new Wt::WContainerWidget(root());
  fixed_header->setStyleClass("fixed-header");
  Wt::WContainerWidget *collapse_in = new Wt::WContainerWidget(fixed_header);
  collapse_in->setStyleClass("collapse in");
  collapse_in->setId("collapseHeader");
  Wt::WContainerWidget *jumbotron = new Wt::WContainerWidget(collapse_in);
  jumbotron->setStyleClass("jumbotron");
  Wt::WContainerWidget *container_head = new Wt::WContainerWidget(jumbotron);
  container_head->setStyleClass("container");
  container_head->addWidget(new Wt::WText("<h1>ANML Viewer</h1>"));
  container_head->addWidget(new Wt::WText("<p>Visualize an automata machine and simulate for a given text file.</p>"));

   /*
   * Error Modal
   */
  Wt::WContainerWidget *error_modal = new Wt::WContainerWidget(root());
  error_modal->setStyleClass("modal fade");
  error_modal->setId("error-modal");
  error_modal->setAttributeValue("tabindex", "-1");
  error_modal->setAttributeValue("role", "dialog");
  error_modal->setAttributeValue("aria-labelledby", "errorModalLabel");
  error_modal->setAttributeValue("aria-hidden", "true");
  Wt::WContainerWidget *error_modal_dialog = new Wt::WContainerWidget(error_modal);
  error_modal_dialog->setStyleClass("modal-dialog modal-sm");
  error_modal_dialog->setAttributeValue("role", "document");
  Wt::WContainerWidget *error_modal_content = new Wt::WContainerWidget(error_modal_dialog);
  error_modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *error_modal_header = new Wt::WContainerWidget(error_modal_content);
  error_modal_header->setStyleClass("modal-header");
  error_modal_header->addWidget(new Wt::WText("<h3 class=\"modal-title\">Simulation Error</h3>"));
  Wt::WContainerWidget *error_modal_body = new Wt::WContainerWidget(error_modal_content);
  error_modal_body->setStyleClass("modal-body");
  Wt::WText *error_modal_message = new Wt::WText(error_modal_body);
  Wt::WContainerWidget *error_modal_footer = new Wt::WContainerWidget(error_modal_content);
  error_modal_footer->setStyleClass("modal-footer");
  Wt::WPushButton *close_error_modal_button = new Wt::WPushButton("Close", error_modal_footer);
  close_error_modal_button->setStyleClass("btn btn-default");
  close_error_modal_button->setAttributeValue("data-dismiss", "modal");

  /* 
   * Input table
   */
  Wt::WTable *input_table = new Wt::WTable();
  input_table->setHeaderCount(1);
  input_table->setStyleClass("input-table");
  // Header row
  input_table->elementAt(0,0)->addWidget(new Wt::WText("Select an automata file (.anml/.mnrl)"));
  input_table->elementAt(0,0)->setStyleClass("input-table-cell");
  input_table->elementAt(0,1)->addWidget(new Wt::WText("Select an input file (.input)"));
  input_table->elementAt(0,1)->setStyleClass("input-table-cell");
  input_table->elementAt(0,2)->addWidget(new Wt::WText("OR Select an ANMLZoo file"));
  input_table->elementAt(0,2)->setStyleClass("input-table-cell");
  simulate = new Wt::WPushButton("Simulate >>");
  simulate->setStyleClass("btn btn-primary btn-lg");
  simulate->clicked().connect( std::bind([=] () {
	// No automata file uploaded
	if (!automataUploaded) {
	  error_modal_message->setText("Automata file must first be uploaded before simulation.");
	  doJavaScript("$('#error-modal').modal('show');");
	}
	// File uploaded is not valid automata
	else if (!validAutomata) {
	  error_modal_message->setText("Cannot simulate over given automata file. \nPlease upload .mnrl or .anml file to simulate.");
	  doJavaScript("$('#error-modal').modal('show');");
	}
	// No input file to simulate on
	else if (!validInput) {
	  error_modal_message->setText("Please upload an input file to simulate on.");
	  doJavaScript("$('#error-modal').modal('show');");
	}
	// All inputs valid
	else {
	  doJavaScript("$('#collapseHeader').collapse()");
	  doJavaScript("toggleChevron()");
	  doJavaScript("$('#sim-tab').show()");
	  doJavaScript("sim_tab_clicked()");
	  ap.enableStartStates();
	  simulateAutomata(simIndex,simIndex,input_string[simIndex]);
	  simulate->setEnabled(false);
	}
	  
      }));
  input_table->elementAt(1,3)->addWidget(simulate);
  input_table->elementAt(1,3)->setStyleClass("input-table-cell");
  input_table->elementAt(1,3)->setRowSpan(2);
  
  // Input row
 
  Wt::WFileUpload *automata_file_upload = new Wt::WFileUpload(input_table->elementAt(1,0));
  automata_file_upload->setFileTextSize(30);
  //automata_file_upload->setFilters(".anml"); v3.3.7
  automata_file_upload->setId("automata_file");

  Wt::WFileUpload *input_file_upload = new Wt::WFileUpload(input_table->elementAt(1,1));
  //input_file_upload->setFilters(".input"); v3.3.7
  input_file_upload->setId("input_file");

  Wt::WComboBox *anmlzoo_combo = new Wt::WComboBox(input_table->elementAt(1,2));
  anmlzoo_combo->setId("anmlzoo-combo");

  anmlzoo_combo->addItem("- Select ANMLZoo File -");
  anmlzoo_combo->addItem("Brill");
  anmlzoo_combo->addItem("ClamAV");
  anmlzoo_combo->addItem("Dotstar");
  anmlzoo_combo->addItem("EntityResolution");
  anmlzoo_combo->addItem("Fermi");
  anmlzoo_combo->addItem("Hamming");
  anmlzoo_combo->addItem("Levenshtein");
  anmlzoo_combo->addItem("PowerEN");
  anmlzoo_combo->addItem("Protomata");
  anmlzoo_combo->addItem("RandomForest");
  anmlzoo_combo->addItem("Snort");
  anmlzoo_combo->addItem("SPM");
  anmlzoo_combo->addItem("Synthetic");
  anmlzoo_combo->setCurrentIndex(0);
  anmlzoo_combo->setMargin(10, Wt::Right);

  anmlzoo_combo->changed().connect(std::bind([=] () {
	if (anmlzoo_combo->currentIndex() > 0)
	  loadDemoGraph(anmlzoo_combo->currentIndex());
      }));

  input_table->elementAt(1,0)->setStyleClass("input-table-cell");
  input_table->elementAt(1,1)->setStyleClass("input-table-cell");
  input_table->elementAt(1,2)->setStyleClass("input-table-cell");
  container_head->addWidget(input_table);

  
  
   
  /*
   * Collapse header button/bar
   */

  Wt::WContainerWidget *collapse_bar = new Wt::WContainerWidget(fixed_header);
  collapse_bar->setStyleClass("collapse-bar");
  Wt::WAnchor *hide_header_btn = new Wt::WAnchor(Wt::WLink("#collapseHeader"), collapse_bar);
  hide_header_btn->setStyleClass("hide-header-btn");
  hide_header_btn->setAttributeValue("data-toggle", "collapse");
  hide_header_btn->setAttributeValue("aria-expanded", "true");
  hide_header_btn->setAttributeValue("aria-controls", "collapseHeader");
  hide_header_btn->setAttributeValue("onClick", "toggleChevron()");
  Wt::WText *glyphicon_chevron = new Wt::WText(hide_header_btn);
  glyphicon_chevron->setStyleClass("glyphicon glyphicon-chevron-up");
  glyphicon_chevron->setId("nav-hide-icon");
  glyphicon_chevron->setAttributeValue("aria-hidden", "true");
  //hide_header_btn->setText("<span id=\"nav-hide-icon\" class=\"glyphicon glyphicon-chevron-up\" aria-hidden=\"true\"></span>");

 

  /*
   * Graph container
   */
  Wt::WContainerWidget *sigma_container = new Wt::WContainerWidget(root());
  sigma_container->setId("container");
  Wt::WContainerWidget *graph_container = new Wt::WContainerWidget(sigma_container);
  graph_container->setId("graph-container");

   // *** MODALS *** //

  /*
   * Loading dialog
   */
  Wt::WContainerWidget *modal = new Wt::WContainerWidget(root());
  modal->setStyleClass("modal bd-example-modal-sm");
  modal->setId("loading-graph-modal");
  modal->setAttributeValue("tabindex", "-1");
  modal->setAttributeValue("role", "dialog");
  modal->setAttributeValue("aria-labelledby", "mySmallModalLabel");
  modal->setAttributeValue("aria-hidden", "true");
  modal->setAttributeValue("data-backdrop", "static");
  Wt::WContainerWidget *modal_dialog = new Wt::WContainerWidget(modal);
  modal_dialog->setStyleClass("modal-dialog modal-sm");
  Wt::WContainerWidget *modal_content = new Wt::WContainerWidget(modal_dialog);
  modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *modal_header = new Wt::WContainerWidget(modal_content);
  modal_header->setStyleClass("modal-header");
  Wt::WText *modal_title = new Wt::WText("<h3 class=\"modal-title\">Please wait</h3>");
  modal_header->addWidget(modal_title);
  Wt::WContainerWidget *modal_body = new Wt::WContainerWidget(modal_content);
  modal_body->setStyleClass("modal-body");
  modal_message->setText("Uploading file...");
  modal_body->addWidget(modal_message);
  
  /*
   * ANML Options dialog
   */
  Wt::WContainerWidget *options_modal = new Wt::WContainerWidget(root());
  options_modal->setStyleClass("modal fade");
  options_modal->setId("options-modal");
  options_modal->setAttributeValue("tabindex", "-1");
  options_modal->setAttributeValue("role", "dialog");
  options_modal->setAttributeValue("aria-labelledby", "options-modal-label");
  Wt::WContainerWidget *options_modal_dialog = new Wt::WContainerWidget(options_modal);
  options_modal_dialog->setStyleClass("modal-dialog");
  options_modal_dialog->setAttributeValue("role", "dialog");
  Wt::WContainerWidget *options_modal_content = new Wt::WContainerWidget(options_modal_dialog);
  options_modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *options_modal_header = new Wt::WContainerWidget(options_modal_content);
  options_modal_header->setStyleClass("modal-header");
  options_modal_header->addWidget(new Wt::WText("<h3 class=\"modal-title\">Automata Constructor Options</h3>"));
  Wt::WContainerWidget *options_modal_body = new Wt::WContainerWidget(options_modal_content);
  options_modal_body->setStyleClass("modal-body");

 

  // Check boxes
  Wt::WContainerWidget *options_modal_fg1 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg1->setStyleClass("form-group");
  Wt::WCheckBox *global_opt_check = new Wt::WCheckBox(" Global Optimization", options_modal_fg1);
  //options_modal_fg1->addWidget(new Wt::WText("<label for='global-opt' class='control-label'>Global Optimization</label>"));
  Wt::WContainerWidget *options_modal_fg2 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg2->setStyleClass("form-group");
  Wt::WCheckBox *local_opt_check = new Wt::WCheckBox(" Local Optimization", options_modal_fg2);
  Wt::WContainerWidget *options_modal_fg3 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg3->setStyleClass("form-group");
  Wt::WCheckBox *remove_or_check = new Wt::WCheckBox(" Remove OR Gates", options_modal_fg3);
    
  // Buttons
  Wt::WContainerWidget *options_modal_footer = new Wt::WContainerWidget(options_modal_content);
  options_modal_footer->setStyleClass("modal-footer");
  Wt::WPushButton *close_options_modal_button = new Wt::WPushButton("Close", options_modal_footer);
  close_options_modal_button->setStyleClass("btn btn-default");
  close_options_modal_button->setAttributeValue("data-dismiss", "modal");
  Wt::WPushButton *generate_options_modal_button = new Wt::WPushButton("Generate", options_modal_footer);
  generate_options_modal_button->setStyleClass("btn btn-primary");
  generate_options_modal_button->clicked().connect(std::bind( [=]() {
	doJavaScript("$('#options-modal').modal('hide');");
	handleAutomataFile(global_opt_check->isChecked(), local_opt_check->isChecked(), remove_or_check->isChecked(), fn);
      }));
    
   

  /*
   * Fixed page footer
   */
  Wt::WContainerWidget *footer = new Wt::WContainerWidget(root());
  footer->setStyleClass("footer");
  // Text input display
  Wt::WContainerWidget *input_display = new Wt::WContainerWidget(footer);
  input_display->setStyleClass("input-display");
  input_display_table = new Wt::WTable(input_display);
  input_display_table->setStyleClass("input-display-table");
  // Nav bar
  std::string navbar_string = 
    std::string("<ul class='nav nav-tabs'>") +
    "<li role='presentation' class='active' id='graph-tab'>" +
    "<a role='button' onClick='graph_tab_clicked()'>Graph Settings" +
    "</a></li>" +
    "<li role='presentation' id='sim-tab' style='display: none'>" +
    "<a role='button' onClick='sim_tab_clicked()'>Simulation Tools" +
    "</a></li>" +
    "</ul>";
  Wt::WText *navbar = new Wt::WText(footer);
  navbar->setTextFormat(Wt::XHTMLUnsafeText);
  navbar->setText(navbar_string);

  // ********************** //
  // *** GRAPH SETTINGS *** //
  // ********************** //

  Wt::WContainerWidget *graph_settings = new Wt::WContainerWidget(footer);
  graph_settings->setId("graph-settings");
  graph_settings->setStyleClass("settings-container");
  // Input Table
  Wt::WTable *footer_table = new Wt::WTable(graph_settings);
  footer_table->setHeaderCount(1);
  footer_table->setStyleClass("footer-table");
  // Table Headers
  footer_table->elementAt(0,0)->addWidget(new Wt::WText("Node Size: "));
  footer_table->elementAt(0,1)->addWidget(new Wt::WText("Arrow Size: "));
  footer_table->elementAt(0,2)->addWidget(new Wt::WText("Edge Thickness: "));
  Wt::WCheckBox *disable_edges = new Wt::WCheckBox(" Render Edges?", footer_table->elementAt(0,3));
  disable_edges->setId("render-edge-box");
  disable_edges->setChecked(true);
  disable_edges->setAttributeValue("onclick", "toggleEdges()");
  footer_table->elementAt(0,4)->addWidget(new Wt::WText("Graph Width"));

  footer_table->elementAt(0,0)->setStyleClass("footer-text");
  footer_table->elementAt(0,1)->setStyleClass("footer-text");
  footer_table->elementAt(0,2)->setStyleClass("footer-text");
  footer_table->elementAt(0,3)->setStyleClass("footer-text");
  footer_table->elementAt(0,4)->setStyleClass("footer-text");

  // Table footers/inputs
  Wt::WSlider *node_slider = new Wt::WSlider(footer_table->elementAt(1,0));
  node_slider->setTickInterval(1);
  node_slider->setMinimum(1);
  node_slider->setMaximum(20);
  node_slider->setValue(4);
  node_slider->setId("node-slider");
  node_slider->valueChanged().connect(std::bind([=] () {
	doJavaScript(("nodeSizeChange(" + node_slider->valueText() + ")").toUTF8());
      }));
  Wt::WSlider *arrow_slider = new Wt::WSlider(footer_table->elementAt(1,1));
  arrow_slider->setTickInterval(1);
  arrow_slider->setMinimum(0);
  arrow_slider->setMaximum(10);
  arrow_slider->setValue(1);
  arrow_slider->setId("arrow-slider");
  arrow_slider->valueChanged().connect(std::bind([=] () {
	doJavaScript(("arrowSizeChange(" + arrow_slider->valueText() + ")").toUTF8());
      }));
  Wt::WSlider *edge_slider = new Wt::WSlider(footer_table->elementAt(1,2));
  edge_slider->setTickInterval(1);
  edge_slider->setMinimum(1);
  edge_slider->setMaximum(15);
  edge_slider->setValue(4);
  edge_slider->setId("edge-slider");
  edge_slider->valueChanged().connect(std::bind([=] () {
	doJavaScript(("edgeSizeChange(" + edge_slider->valueText() + ")").toUTF8());
      }));
  Wt::WSlider *x_scale_slider = new Wt::WSlider(footer_table->elementAt(1,4));
  x_scale_slider->setTickInterval(5);
  x_scale_slider->setMinimum(5);
  x_scale_slider->setMaximum(100);
  x_scale_slider->setValue(10);
  x_scale_slider->setId("x-scale-slider");
  x_scale_slider->valueChanged().connect(std::bind([=] () {
	doJavaScript(("xScaleChange(" + x_scale_slider->valueText() + ")").toUTF8());
      }));
  footer_table->elementAt(1,0)->setStyleClass("footer-text");
  footer_table->elementAt(1,1)->setStyleClass("footer-text");
  footer_table->elementAt(1,2)->setStyleClass("footer-text");
  footer_table->elementAt(1,4)->setStyleClass("footer-text");
  

  // ************************ //
  // *** SIMULATION TOOLS *** //
  // ************************ //
  
  Wt::WContainerWidget *simulation_tools = new Wt::WContainerWidget(footer);
  simulation_tools->setId("sim-tools");
  simulation_tools->setStyleClass("settings-container");
  simulation_tools->hide();
  sim_step = new Wt::WPushButton("Step >>", simulation_tools);
  sim_step->setStyleClass("btn btn-primary");
  sim_step->setId("sim-step-btn");
  sim_step->clicked().connect( std::bind( [=] () {
	if (simIndex < input_string.length() - 1) {
	  simulateAutomata(simIndex, simIndex+1, input_string[simIndex+1]);
	  simIndex++;
	}
      }));
  Wt::WPushButton *play_btn = new Wt::WPushButton("Play Simulation", simulation_tools);
  play_btn->setStyleClass("btn btn-primary");
  play_btn->setId("play-sim-btn");
  play_btn->clicked().connect( std::bind( [=] () {
	if (play_btn->text() == "Play Simulation") {
	  playSim = true;
	  play_btn->setText("Stop Simulation");
	}
	else {
	  playSim = false;
	  play_btn->setText("Play Simulation");
	}
	while (playSim) {
	  if (simIndex < input_string.length() - 1) {
	    simulateAutomata(simIndex, simIndex+1, input_string[simIndex+1]);
	    simIndex++;
	    processEvents();
	  }
	  else {
	    playSim = false;
	  }
	}
      }));
  

  // Include scripts
  Wt::WApplication::instance()->requireJQuery("https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js");
  Wt::WApplication::instance()->require("https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js");
  Wt::WApplication::instance()->require("js/script.js");

  // Upload File Process
  //
  // Uploading automata file
  automata_file_upload->changed().connect( std::bind([=] () {
	modal_message->setText("Uploading file...");
	std::cout << "STATUS: UPLOADING AUTOMATA FILE" << std::endl;
	newFileUploaded();
	doJavaScript("$('#loading-graph-modal').modal('show');");
	automata_file_upload->upload();
      }));
  // Converting file and displaying graph
  automata_file_upload->uploaded().connect(std::bind([=] () {
	std::vector< Wt::Http::UploadedFile > file_vector = automata_file_upload->uploadedFiles();
	std::string filename = automata_file_upload->clientFileName().toUTF8();
	fn = file_vector.data()->spoolFileName();
	automataUploaded = true;

	// Handle pre-formatted JSON files
	if (filename.substr(filename.find_last_of(".")) == ".json") {
	  std::ifstream ifs(fn);
	  std::string content( (std::istreambuf_iterator<char>(ifs) ),
			       (std::istreambuf_iterator<char>()    ) );
	  std::string json_string = content;
	  modal_message->setText("Loading graph...");
	  processEvents();
	  std::cout << "STATUS: LOADING GRAPH IN SIGMA" << std::endl;
	  validAutomata = false;
	  doJavaScript("loadGraph("+json_string+")");
	}
	// Handle MNRL and ANML
	else if (filename.substr(filename.find_last_of(".")) == ".mnrl" ||
		 filename.substr(filename.find_last_of(".")) == ".anml") {

	  // Displays option modal
	  doJavaScript("$('#loading-graph-modal').modal('hide');");
	  doJavaScript("$('#options-modal').modal('show');");
	}
	// Unsupported file type
	else {
	  modal_message->setText("File type unsupported.");
	  modal_title->setText("<h3 class=\"modal-title\">Error creating graph</h3>");
	  validAutomata = false;
	}
	
	// Writing json to local temp file
	//std::ofstream out("output.txt");
	//out << json_string;
	//out.close();
	
      }));

  // Uploading input file
  input_file_upload->changed().connect( std::bind([=] () {
	modal_message->setText("Uploading file...");
	std::cout << "STATUS: UPLOADING INPUT FILE" << std::endl;
	doJavaScript("$('#loading-graph-modal').modal('show')");
	newFileUploaded();
	input_file_upload->upload();
      }));

  // Handle uploaded file
  input_file_upload->uploaded().connect(std::bind([=] () {
	
	doJavaScript("$('#loading-graph-modal').modal('hide');");
	// doJavaScript("$('#input-table-row').html(\"\");");
	// Read file contents into global string 'input'
	std::vector< Wt::Http::UploadedFile > file_vector = input_file_upload->uploadedFiles();
	loadTextFile(""+file_vector.data()->spoolFileName());

      }));

}
/*
 * Resets simulation settings when new automata or input file is uploaded
 */
void VASimViz::newFileUploaded() {
  playSim = false;
  input_display_table->elementAt(0, simIndex)->setStyleClass("");
  simIndex = 0;
  doJavaScript("graph_tab_clicked()");
  doJavaScript("$('#sim-tab').hide()");
  simulate->setEnabled(true);
}

void VASimViz::loadTextFile(std::string fn) {
  std::cout << "STATUS: READING FILE AND GENERATING TABLE" << std::endl;
  input_string = "";
  std::ifstream myfile (fn);
  std::string line;
  while ( getline (myfile, line) ){
    input_string += line;
  }
  myfile.close();
  std::cout << "Read file correctly " << std::endl;

  // Add up to 1000 table cells total, each containing a character
  int length = (input_string.length() > 1000) ? 1000 : input_string.length();
  input_display_table->clear();
  for (int i = 0; i < length; i++) {
    input_display_table->elementAt(0,i)->addWidget(new Wt::WText(std::string(1, input_string[i])));
  }

  validInput = true;
}

void VASimViz::handleAutomataFile(bool global, bool local, bool OR, std::string fn) 
{
  // Create Automata object from file
  std::cout << "STATUS: CREATING AUTOMATA OBJECT FROM FILE" << std::endl;
  modal_message->setText("Creating Automata object from file...");
  doJavaScript("$('#loading-graph-modal').modal('show')");
  processEvents();
  Automata a(fn);
  ap = a;
  validAutomata = true;
  uint32_t automata_size = ap.getElements().size();
  uint32_t orig_automata_size = ap.getElements().size();
  
  modal_message->setText("Processing optimizations...");
  processEvents();

  // Optimizations
  if (global) {
    ap.leftMinimize();
        
    while(automata_size != ap.getElements().size()) {
      automata_size = ap.getElements().size();
      ap.leftMinimize();
    }
  }
  if (OR) {
    ap.removeOrGates();
  }

  // Convert to JSON
  modal_message->setText("Converting to JSON format...");
  processEvents();
  std::cout << "STATUS: CONVERTING AUTOMATA TO JSON" << std::endl;
  std::string json_string = SigmaJSONWriter::writeToJSON(&ap);

  // Load JSON in graph
  modal_message->setText("Loading graph...");
  processEvents();
  std::cout << "STATUS: LOADING GRAPH IN SIGMA" << std::endl;
  doJavaScript("loadGraph("+json_string+")");
  
}

void VASimViz::simulateAutomata(int prevIndex, int index, char symbol) {
  //  std::cout << "Simulating on '" << symbol << "'" << std::endl;
  std::string json_out = SigmaJSONWriter::simulateStep(&ap, symbol);
  input_display_table->elementAt(0, prevIndex)->setStyleClass("");
  input_display_table->elementAt(0, index)->setStyleClass("highlight");
  doJavaScript("updateGraph(" + json_out  + ")");
}

void VASimViz::loadDemoGraph(int index) {
  // Allows for simulation button to be clicked
  automataUploaded = true;
  validAutomata = true;
  validInput = true;

  newFileUploaded(); 

  switch (index) {
  case 1:
    loadTextFile("VASim/ANMLZoo/Brill/inputs/brill_1MB.input");
    fn = "VASim/ANMLZoo/Brill/anml/brill.1chip.anml";
    break;
  case 2:
    loadTextFile("VASim/ANMLZoo/ClamAV/inputs/vasim_1MB.input");
    fn = "VASim/ANMLZoo/ClamAV/anml/515_nocounter.1chip.anml";
    break;
  case 3:
    loadTextFile("VASim/ANMLZoo/Dotstar/inputs/backdoor_1MB.input");
    fn = "VASim/ANMLZoo/Dotstar/anml/backdoor_dotstar.1chip.anml";
    break;
  case 4:
    loadTextFile("VASim/ANMLZoo/EntityResolution/inputs/1000_1MB.input");
    fn = "VASim/ANMLZoo/EntityResolution/anml/1000.1chip.anml";
    break;
  case 5:
    loadTextFile("VASim/ANMLZoo/Fermi/inputs/rp_input_1MB.input");
    fn = "VASim/ANMLZoo/Fermi/anml/fermi_2400.1chip.anml";
    break;
  case 6:
    loadTextFile("VASim/ANMLZoo/Hamming/inputs/hamming_1MB.input");
    fn = "VASim/ANMLZoo/Hamming/anml/93_20X3.1chip.anml";
    break;
  case 7:
    loadTextFile("VASim/ANMLZoo/Levenshtein/inputs/DNA_1MB.input");
    fn = "VASim/ANMLZoo/Levenshtein/anml/24_20x3.1chip.anml";
    break;
  case 8:
    loadTextFile("VASim/ANMLZoo/PowerEN/inputs/poweren_1MB.input");
    fn = "VASim/ANMLZoo/PowerEN/anml/complx_010000_00123.1chip.anml";
    break;
  case 9:
    loadTextFile("VASim/ANMLZoo/Protomata/inputs/uniprot_fasta_1MB.input");
    fn = "VASim/ANMLZoo/Protomata/anml/2340sigs.1chip.anml";
    break;
  case 10:
    loadTextFile("VASim/ANMLZoo/RandomForest/inputs/mnist_1MB.input");
    fn = "VASim/ANMLZoo/RandomForest/anml/300f_15t_tree_from_model_MNIST.anml";
    break;
  case 11:
    loadTextFile("VASim/ANMLZoo/Snort/inputs/snort_1MB.input");
    fn = "VASim/ANMLZoo/Snort/anml/snort.1chip.anml";
    break;
  case 12:
    loadTextFile("VASim/ANMLZoo/SPM/inputs/SPM_1MB.input");
    fn = "VASim/ANMLZoo/SPM/anml/bible_size4.1chip.anml";
    break;
  case 13:
    loadTextFile("VASim/ANMLZoo/Synthetic/inputs/1MB.input");
    fn = "VASim/ANMLZoo/Synthetic/anml/BlockRings.anml";
    break;
  }
  doJavaScript("$('#loading-graph-modal').modal('hide');");
  doJavaScript("$('#options-modal').modal('show');");
}

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new VASimViz(env);
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
