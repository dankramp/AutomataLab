#include <Wt/Http/Client>
#include <Wt/Http/Request>
#include <Wt/WApplication>
#include <Wt/WAnchor>
#include <Wt/WBreak>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WCheckBox>
#include <Wt/WDialog>
#include <Wt/WEnvironment>
#include <Wt/WFileUpload>
#include <Wt/WIntValidator>
#include <Wt/WJavaScript>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WLink>
#include <Wt/WPushButton>
#include <Wt/WSlider>
#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WTableRow>
#include <Wt/WTextArea>

#include <iostream>
#include <iomanip>
#include <fstream>

#include "automata.h"
#include "SigmaJSONWriter.h"
#include "RandomGraph.h"
#include <json11.hpp>

class VASimViz : public Wt::WApplication
{
public:
  VASimViz(const Wt::WEnvironment& env);
  void newFileUploaded();
  void loadTextFromFile(std::string fn);
  void loadInputTable();
  void handleAutomataFile(bool global, bool OR, std::string fn, int32_t fanin_limit=-1, int32_t fanout_limit=-1);
  void loadRandomAutomata();
  std::string simulateAutomata(char symbol);
  void beginSimulation();
  void addToJSCache(int numGraphs);
  void loadDemoGraph(std::string name, bool user);
  void toggleConnection(std::string sourceId, std::string targetId, bool addEdge);
  void changeSTEData(std::string id, std::string ss, std::string start, std::string rep);
  void deleteSTE(std::string id);
 
  Wt::JSignal<std::string, std::string, bool> toggleConnection_;
  Wt::JSignal<std::string, std::string, std::string, std::string> changeSTEData_;
  Wt::JSignal<std::string> deleteSTE_;

private:
  std::string fn;
  std::string input_string;
  const int max_input_display_size = 1000;

  Wt::WText *load_modal_message = new Wt::WText();
  Wt::WTable *input_display_table;
  Wt::WPushButton *sim_step;
  Wt::WPushButton *sim_rev;
  Wt::WPushButton *simulate;
  Wt::WComboBox *anmlzoo_combo;
  Wt::WText *error_modal_message;

  Wt::WText *ste_options_title;
  Wt::WLineEdit *ste_id_input;
  Wt::WLineEdit *ste_ss_input;
  Wt::WLabel *ste_rep_label;
  Wt::WCheckBox *reporting_check;
  Wt::WLineEdit *ste_rep_input;
  Wt::WComboBox *ste_start_select;
  Wt::WPushButton *ste_options_create_btn;
  std::string ste_id;
  Wt::WText *delete_modal_text;

  Automata ap;
  json11::Json cache = json11::Json::array{};

  bool automataUploaded = false;
  bool validAutomata = false;
  bool validInput = false;
  bool editingSTE = false;
  int cache_index = -1;
  // How many new items to add to cache
  int cache_fetch_size = 100;

};

VASimViz::VASimViz(const Wt::WEnvironment& env)
  : Wt::WApplication(env),
    toggleConnection_(this, "toggleConnection"),
    changeSTEData_(this, "changeSTEData"),
    deleteSTE_(this, "deleteSTE")
{

  toggleConnection_.connect(this, &VASimViz::toggleConnection);
  changeSTEData_.connect(this, &VASimViz::changeSTEData);
  deleteSTE_.connect(this, &VASimViz::deleteSTE);

  // Set page title
  setTitle("ANML Viewer");

  /* STYLE SHEETS */

  Wt::WApplication::instance()->useStyleSheet("https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css");
  Wt::WApplication::instance()->useStyleSheet("styles.css");
  
  /* IMPORTS */
  // Some are from sigma.js, some from linkurious.js, because linkurious plugins
  // require some core to be from itself, but some parts of sigma core are faster
  
  // Core
  Wt::WApplication::instance()->require("/sigma.js/src/sigma.core.js");
  Wt::WApplication::instance()->require("/sigma.js/src/conrad.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/utils/sigma.utils.js");
  Wt::WApplication::instance()->require("/sigma.js/src/utils/sigma.polyfills.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/sigma.settings.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/classes/sigma.classes.dispatcher.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.configurable.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.graph.js"); // this makes RandomForest fast/slow
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.camera.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/classes/sigma.classes.quad.js");
  Wt::WApplication::instance()->require("/sigma.js/src/classes/sigma.classes.edgequad.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/captors/sigma.captors.mouse.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/captors/sigma.captors.touch.js");

  // Renderers
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/sigma.renderers.canvas.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/sigma.renderers.webgl.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/sigma.renderers.def.js");

  // WebGL 
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.nodes.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.nodes.fast.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.edges.def.js");
  Wt::WApplication::instance()->require("/sigma.js/src/renderers/webgl/sigma.webgl.edges.arrow.js");

  // Canvas
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.labels.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.hovers.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.nodes.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.edges.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.edges.arrow.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.edgehovers.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/renderers/canvas/sigma.canvas.extremities.def.js");

  // Middlewares and misc
  Wt::WApplication::instance()->require("/linkurious.js/src/middlewares/sigma.middlewares.rescale.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/middlewares/sigma.middlewares.copy.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/misc/sigma.misc.animation.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/misc/sigma.misc.bindEvents.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/misc/sigma.misc.bindDOMEvents.js");
  Wt::WApplication::instance()->require("/linkurious.js/src/misc/sigma.misc.drawHovers.js");

  // Linkurious plugins
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.dragNodes/sigma.plugins.dragNodes.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.helpers.graph/sigma.helpers.graph.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.activeState/sigma.plugins.activeState.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.select/sigma.plugins.select.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.keyboard/sigma.plugins.keyboard.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.lasso/sigma.plugins.lasso.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.rectSelect/sigma.plugins.rectSelect.js");

  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.locate/sigma.plugins.locate.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.plugins.tooltips/sigma.plugins.tooltips.js");

  // Linkurious renderer
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.renderers.linkurious/settings.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.labels.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.hovers.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.def.js");
  Wt::WApplication::instance()->require("/linkurious.js/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.arrow.js");

  // JQuery
  Wt::WApplication::instance()->requireJQuery("https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js");
  // Bootstrap
  Wt::WApplication::instance()->require("https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js");
  // Mustache
  Wt::WApplication::instance()->require("https://cdnjs.cloudflare.com/ajax/libs/mustache.js/0.8.1/mustache.min.js");  
  // FileSaver
  Wt::WApplication::instance()->require("FileSaver.js/FileSaver.js");  
  // Custom page scripts
  Wt::WApplication::instance()->require("js/Viewer-Settings.js");
  Wt::WApplication::instance()->require("js/page_script.js");
  Wt::WApplication::instance()->require("js/sigma_script.js"); 


  // Full page container
  Wt::WContainerWidget *full_page_container = new Wt::WContainerWidget(root());
  full_page_container->setId("page-container");



  // ******************* //
  // *** PAGE HEADER *** //
  // ******************* //


  Wt::WContainerWidget *fixed_header = new Wt::WContainerWidget(full_page_container);
  fixed_header->setStyleClass("fixed-header");
  fixed_header->setId("header");
  Wt::WContainerWidget *collapse_in = new Wt::WContainerWidget(fixed_header);
  collapse_in->setStyleClass("collapse in");
  collapse_in->setId("collapseHeader");
  Wt::WContainerWidget *jumbotron = new Wt::WContainerWidget(collapse_in);
  jumbotron->setStyleClass("jumbotron");
  Wt::WContainerWidget *container_head = new Wt::WContainerWidget(jumbotron);
  container_head->setStyleClass("container");
  container_head->addWidget(new Wt::WText("<h1>ANML Viewer</h1>"));
  container_head->addWidget(new Wt::WText("<p>Visualize an automata machine and simulate for a given text file.</p>"));
  Wt::WTable *input_table = new Wt::WTable(container_head);
  input_table->setHeaderCount(1);
  input_table->setStyleClass("header-table");

  /* Table Headers */

  input_table->elementAt(0,0)->addWidget(new Wt::WText("Upload an automata file (.anml/.mnrl)"));
  input_table->elementAt(0,1)->addWidget(new Wt::WText("Upload an input file or  <a role='button' data-toggle='modal' data-target='#text-input-modal'>type input</a>", Wt::XHTMLUnsafeText));
  input_table->elementAt(0,2)->addWidget(new Wt::WText("OR Select an ANMLZoo file"));
  
  /* Input row */
 
  Wt::WFileUpload *automata_file_upload = new Wt::WFileUpload(input_table->elementAt(1,0));
  automata_file_upload->setFileTextSize(30);
  //automata_file_upload->setFilters(".anml"); v3.3.7
  automata_file_upload->setId("automata_file");

  Wt::WFileUpload *input_file_upload = new Wt::WFileUpload(input_table->elementAt(1,1));
  //input_file_upload->setFilters(".input"); v3.3.7
  input_file_upload->setId("input_file");

  anmlzoo_combo = new Wt::WComboBox(input_table->elementAt(1,2));
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
  anmlzoo_combo->addItem("Donut");
  //anmlzoo_combo->addItem("Random");
  anmlzoo_combo->setCurrentIndex(0);
  anmlzoo_combo->setMargin(10, Wt::Right);
  anmlzoo_combo->changed().connect(std::bind([=] () {
	if (anmlzoo_combo->currentIndex() > 0)
	  loadDemoGraph(anmlzoo_combo->currentText().toUTF8(), true);
      }));

  simulate = new Wt::WPushButton("Simulate <span class='glyphicon glyphicon-forward' aria-hidden='true'></span>");
  simulate->setTextFormat(Wt::XHTMLUnsafeText);
  simulate->setStyleClass("btn btn-primary btn-lg");
  simulate->clicked().connect( std::bind([=] () {
	if (ap.getElements().size() < 1) { // Automata is empty
	  error_modal_message->setText("Automata has no elements. Please upload a file or create an automata before simulating.");
	  doJavaScript("$('#error-modal').modal('show');");
	} else if (!validInput) { // No input file to simulate on
	  error_modal_message->setText("Please upload an input file to simulate on.");
	  doJavaScript("$('#error-modal').modal('show');");
	} else { // All inputs valid
	  doJavaScript("$('#collapseHeader').collapse()");
	  doJavaScript("toggleChevron()");
	  doJavaScript("$('#sim-tab').show()");
	  doJavaScript("sim_tab_clicked()");
	  beginSimulation();
	  simulate->setEnabled(false);
	}
      }));
  input_table->elementAt(1,3)->addWidget(simulate);
  //input_table->elementAt(1,3)->setRowSpan(2);
   
  /* Collapse header button/bar */

  Wt::WContainerWidget *collapse_bar = new Wt::WContainerWidget(fixed_header);
  collapse_bar->setStyleClass("collapse-bar");
  collapse_bar->setId("click-collapse-bar");
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


  // ************** //
  // *** MODALS *** //
  // ************** //


  /* Error Modal */

  Wt::WContainerWidget *error_modal = new Wt::WContainerWidget(full_page_container);
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
  error_modal_header->addWidget(new Wt::WText("<h3 class=\"modal-title\">Error</h3>"));
  Wt::WContainerWidget *error_modal_body = new Wt::WContainerWidget(error_modal_content);
  error_modal_body->setStyleClass("modal-body");
  error_modal_message = new Wt::WText(error_modal_body);
  Wt::WContainerWidget *error_modal_footer = new Wt::WContainerWidget(error_modal_content);
  error_modal_footer->setStyleClass("modal-footer");
  Wt::WPushButton *close_error_modal_button = new Wt::WPushButton("Close", error_modal_footer);
  close_error_modal_button->setStyleClass("btn btn-default");
  close_error_modal_button->setAttributeValue("data-dismiss", "modal");

  /* Type Input Modal */

  Wt::WContainerWidget *text_input_modal = new Wt::WContainerWidget(full_page_container);
  text_input_modal->setStyleClass("modal fade");
  text_input_modal->setId("text-input-modal");
  text_input_modal->setAttributeValue("tabindex", "-1");
  text_input_modal->setAttributeValue("role", "dialog");
  Wt::WContainerWidget *text_input_dialog = new Wt::WContainerWidget(text_input_modal);
  text_input_dialog->setStyleClass("modal-dialog");
  text_input_dialog->setAttributeValue("role", "document");
  Wt::WContainerWidget *text_input_content = new Wt::WContainerWidget(text_input_dialog);
  text_input_content->setStyleClass("modal-content");
  Wt::WContainerWidget *text_input_header = new Wt::WContainerWidget(text_input_content);
  text_input_header->setStyleClass("modal-header");
  text_input_header->addWidget(new Wt::WText("<h3 class='modal-title'>Enter input stream</h3>"));
  Wt::WContainerWidget *text_input_body = new Wt::WContainerWidget(text_input_content);
  text_input_body->setStyleClass("modal-body");
  Wt::WContainerWidget *text_input_form_group = new Wt::WContainerWidget(text_input_body);
  text_input_form_group->setStyleClass("form-group");
  Wt::WTextArea *text_input_area = new Wt::WTextArea(text_input_form_group);
  text_input_area->setStyleClass("form-control");
  Wt::WContainerWidget *text_input_footer = new Wt::WContainerWidget(text_input_content);
  text_input_footer->setStyleClass("modal-footer");
  Wt::WPushButton *close_text_input_btn = new Wt::WPushButton("Close", text_input_footer);
  close_text_input_btn->setStyleClass("btn btn-default");
  close_text_input_btn->setAttributeValue("data-dismiss", "modal");
  Wt::WPushButton *submit_text_input_btn = new Wt::WPushButton("Load Text", text_input_footer);
  submit_text_input_btn->setStyleClass("btn btn-primary");
  submit_text_input_btn->setAttributeValue("data-dismiss", "modal");
  submit_text_input_btn->clicked().connect(std::bind( [=]() {
	input_string = text_input_area->text().toUTF8();
	loadInputTable();
	text_input_area->setText("");
      }));

  /* Simulating Cache modal */

  Wt::WContainerWidget *small_modal = new Wt::WContainerWidget(full_page_container);
  small_modal->setStyleClass("modal bs-example-modal-sm");
  small_modal->setId("cache-modal");
  small_modal->setAttributeValue("role", "dialog");
  small_modal->setAttributeValue("data-backdrop", "false");
  small_modal->setAttributeValue("aria-hidden", "true");
  small_modal->setAttributeValue("tabindex", "-1");
  Wt::WContainerWidget *small_modal_dialog = new Wt::WContainerWidget(small_modal);
  small_modal_dialog->setStyleClass("modal-dialog modal-sm");
  small_modal_dialog->setAttributeValue("role", "document");
  Wt::WContainerWidget *small_modal_content = new Wt::WContainerWidget(small_modal_dialog);
  small_modal_content->setStyleClass("modal-content");
  small_modal_content->addWidget(new Wt::WText("Simulating automata into cache..."));

  /* Search By ID Modal */

  Wt::WContainerWidget *search_modal = new Wt::WContainerWidget(full_page_container);  
  search_modal->setStyleClass("modal bs-example-modal-sm");
  search_modal->setId("search-modal");
  search_modal->setAttributeValue("role", "dialog");
  search_modal->setAttributeValue("aria-hidden", "true");
  search_modal->setAttributeValue("tabindex", "-1");
  Wt::WContainerWidget *search_modal_dialog = new Wt::WContainerWidget(search_modal);
  search_modal_dialog->setStyleClass("modal-dialog modal-sm");
  search_modal_dialog->setAttributeValue("role", "document");
  Wt::WContainerWidget *search_modal_content = new Wt::WContainerWidget(search_modal_dialog);
  search_modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *search_modal_header = new Wt::WContainerWidget(search_modal_content);
  search_modal_header->setStyleClass("modal-header");
  search_modal_header->addWidget(new Wt::WText("<h3 class='modal-title'>Search By ID</h3>"));
  Wt::WContainerWidget *search_modal_body = new Wt::WContainerWidget(search_modal_content);
  search_modal_body->setStyleClass("modal-body");
  Wt::WContainerWidget *search_modal_input_group = new Wt::WContainerWidget(search_modal_body);
  search_modal_input_group->setStyleClass("input-group");
  Wt::WLineEdit *search_bar = new Wt::WLineEdit(search_modal_input_group);
  search_bar->setStyleClass("form-control");
  search_bar->setId("search-bar");
  search_bar->setAttributeValue("placeholder", "Search by ID...");
  search_bar->enterPressed().connect(std::bind( [=]() {
	doJavaScript("$('#search-btn').click()");
      }));
  Wt::WText *search_button = new Wt::WText("<button class='btn btn-secondary' id='search-btn' onClick='searchById()' data-dismiss='modal' type='button'>Search</button>", Wt::XHTMLUnsafeText, search_modal_input_group);
  search_button->setStyleClass("input-group-btn");

  /* Delete By ID Modal */

  Wt::WContainerWidget *delete_modal = new Wt::WContainerWidget(full_page_container);  
  delete_modal->setStyleClass("modal bs-example-modal-sm");
  delete_modal->setId("delete-ste-modal");
  delete_modal->setAttributeValue("role", "dialog");
  delete_modal->setAttributeValue("aria-hidden", "true");
  delete_modal->setAttributeValue("tabindex", "-1");
  Wt::WContainerWidget *delete_modal_dialog = new Wt::WContainerWidget(delete_modal);
  delete_modal_dialog->setStyleClass("modal-dialog modal-sm");
  delete_modal_dialog->setAttributeValue("role", "document");
  Wt::WContainerWidget *delete_modal_content = new Wt::WContainerWidget(delete_modal_dialog);
  delete_modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *delete_modal_header = new Wt::WContainerWidget(delete_modal_content);
  delete_modal_header->setStyleClass("modal-header");
  delete_modal_header->addWidget(new Wt::WText("<h3 class='modal-title'>Confirm Delete</h3>"));
  Wt::WContainerWidget *delete_modal_body = new Wt::WContainerWidget(delete_modal_content);
  delete_modal_body->setStyleClass("modal-body");
  delete_modal_text = new Wt::WText("Are you sure you want to delete this STE?", delete_modal_body);
  Wt::WContainerWidget *delete_modal_footer = new Wt::WContainerWidget(delete_modal_content);
  delete_modal_footer->setStyleClass("modal-footer");
  Wt::WPushButton *delete_modal_cancel_btn = new Wt::WPushButton("No", delete_modal_footer);
  delete_modal_cancel_btn->setStyleClass("btn btn-default");
  delete_modal_cancel_btn->setAttributeValue("data-dismiss", "modal");
  Wt::WPushButton *delete_modal_confirm_btn = new Wt::WPushButton("Yes", delete_modal_footer);
  delete_modal_confirm_btn->setId("delete-ste-btn");
  delete_modal_confirm_btn->setStyleClass("btn btn-danger");
  delete_modal_confirm_btn->setAttributeValue("data-dismiss", "modal");
  delete_modal_confirm_btn->clicked().connect(std::bind( [=]() {
	std::cout << "id: '" << ste_id << "'" << std::endl;
	ap.removeElement(ap.getElements()[ste_id]);
	doJavaScript("deleteSTE('" + ste_id + "')");
      }));

  /* Loading modal */

  Wt::WContainerWidget *load_modal = new Wt::WContainerWidget(full_page_container);
  load_modal->setStyleClass("modal bd-example-modal-sm");
  load_modal->setId("loading-graph-modal");
  load_modal->setAttributeValue("tabindex", "-1");
  load_modal->setAttributeValue("role", "dialog");
  load_modal->setAttributeValue("aria-labelledby", "mySmallModalLabel");
  load_modal->setAttributeValue("aria-hidden", "true");
  load_modal->setAttributeValue("data-backdrop", "static");
  Wt::WContainerWidget *load_modal_dialog = new Wt::WContainerWidget(load_modal);
  load_modal_dialog->setStyleClass("modal-dialog modal-sm");
  Wt::WContainerWidget *load_modal_content = new Wt::WContainerWidget(load_modal_dialog);
  load_modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *load_modal_header = new Wt::WContainerWidget(load_modal_content);
  load_modal_header->setStyleClass("modal-header");
  Wt::WText *load_modal_title = new Wt::WText("<h3 class=\"modal-title\">Please wait</h3>");
  load_modal_header->addWidget(load_modal_title);
  Wt::WContainerWidget *load_modal_body = new Wt::WContainerWidget(load_modal_content);
  load_modal_body->setStyleClass("modal-body");
  load_modal_message->setText("Uploading file...");
  load_modal_body->addWidget(load_modal_message);
  
  /* ANML Construction Options modal */

  Wt::WContainerWidget *options_modal = new Wt::WContainerWidget(full_page_container);
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

  Wt::WContainerWidget *options_modal_fg1 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg1->setStyleClass("form-group");
  Wt::WCheckBox *global_opt_check = new Wt::WCheckBox(" Global Optimization", options_modal_fg1);
  Wt::WContainerWidget *options_modal_fg2 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg2->setStyleClass("form-group");
  Wt::WCheckBox *remove_or_check = new Wt::WCheckBox(" Remove OR Gates", options_modal_fg2);
  Wt::WContainerWidget *options_modal_fg3 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg3->setStyleClass("form-group");
  Wt::WText *fan_in_text = new Wt::WText("<b>Fan-In Limit: </b>", options_modal_fg3);
  Wt::WLineEdit *fan_in_input = new Wt::WLineEdit(options_modal_fg3);
  Wt::WText *fan_in_error = new Wt::WText(options_modal_fg3);
  fan_in_error->setStyleClass("error-text");
  Wt::WIntValidator *int_validator = new Wt::WIntValidator();
  int_validator->setBottom(1);
  fan_in_input->setValidator(int_validator);
  Wt::WContainerWidget *options_modal_fg4 = new Wt::WContainerWidget(options_modal_body);
  options_modal_fg4->setStyleClass("form-group");
  Wt::WText *fan_out_text = new Wt::WText("<b>Fan-Out Limit: </b>", options_modal_fg4);
  Wt::WLineEdit *fan_out_input = new Wt::WLineEdit(options_modal_fg4);
  Wt::WText *fan_out_error = new Wt::WText(options_modal_fg4);
  fan_out_error->setStyleClass("error-text");
  fan_out_input->setValidator(int_validator);
    
  Wt::WContainerWidget *options_modal_footer = new Wt::WContainerWidget(options_modal_content);
  options_modal_footer->setStyleClass("modal-footer");
  Wt::WPushButton *close_options_modal_button = new Wt::WPushButton("Close", options_modal_footer);
  close_options_modal_button->setStyleClass("btn btn-default");
  close_options_modal_button->setAttributeValue("data-dismiss", "modal");
  close_options_modal_button->clicked().connect(std::bind ( [=]() {
	validAutomata = false;
      }));
  Wt::WPushButton *generate_options_modal_button = new Wt::WPushButton("Generate", options_modal_footer);
  generate_options_modal_button->setStyleClass("btn btn-primary");
  generate_options_modal_button->clicked().connect(std::bind( [=]() {
	if (fan_in_input->validate() != Wt::WValidator::Valid)
	  fan_in_error->setText(" * Enter a valid number > 0");
	else
	  fan_in_error->setText("");
	if (fan_out_input->validate() != Wt::WValidator::Valid)
	  fan_out_error->setText(" * Enter a valid number > 0");
	else
	  fan_out_error->setText("");

	if (fan_in_input->validate() == Wt::WValidator::Valid && fan_out_input->validate() == Wt::WValidator::Valid) {
	  doJavaScript("$('#options-modal').modal('hide');");
	  int fan_in = (fan_in_input->text().toUTF8().length() > 0) ? std::stoi(fan_in_input->text().toUTF8()) : 0;
	  int fan_out = (fan_out_input->text().toUTF8().length() > 0) ? std::stoi(fan_out_input->text().toUTF8()) : 0;
	  handleAutomataFile(global_opt_check->isChecked(), remove_or_check->isChecked(), fn, fan_in, fan_out);
	  fan_in_error->setText("");
	  fan_out_error->setText("");
	  fan_in_input->setText("");
	  fan_out_input->setText("");
	  global_opt_check->setCheckState(Wt::CheckState::Unchecked);
	  remove_or_check->setCheckState(Wt::CheckState::Unchecked);
	}
      }));

  /* Add STE Dialog */

  Wt::WContainerWidget *ste_options_modal = new Wt::WContainerWidget(full_page_container);
  ste_options_modal->setId("add-ste-modal");
  ste_options_modal->setStyleClass("modal fade");
  ste_options_modal->setAttributeValue("tabindex", "-1");
  ste_options_modal->setAttributeValue("role", "dialog");
  ste_options_modal->setAttributeValue("aria-hidden", "true");
  ste_options_modal->setAttributeValue("data-backdrop", "static");
  Wt::WContainerWidget *ste_options_modal_dialog = new Wt::WContainerWidget(ste_options_modal);
  ste_options_modal_dialog->setStyleClass("modal-dialog");
  ste_options_modal_dialog->setAttributeValue("role", "document");
  Wt::WContainerWidget *ste_options_modal_content = new Wt::WContainerWidget(ste_options_modal_dialog);
  ste_options_modal_content->setStyleClass("modal-content");
  Wt::WContainerWidget *ste_options_modal_header = new Wt::WContainerWidget(ste_options_modal_content);
  ste_options_modal_header->setStyleClass("modal-header");
  ste_options_title = new Wt::WText("<h3 class='modal-title'>Create STE</h3>", ste_options_modal_header);
  Wt::WContainerWidget *ste_options_modal_body = new Wt::WContainerWidget(ste_options_modal_content);
  ste_options_modal_body->setStyleClass("modal-body");

  Wt::WTable *ste_options_modal_table = new Wt::WTable(ste_options_modal_body);
  ste_options_modal_table->setStyleClass("ste-options-table");

  Wt::WContainerWidget *ste_options_fg1 = new Wt::WContainerWidget(ste_options_modal_table->elementAt(0,0));
  ste_options_fg1->addStyleClass("form-group");
  Wt::WLabel *ste_id_label = new Wt::WLabel("ID: ", ste_options_fg1);
  ste_id_label->setStyleClass("control-label");
  ste_id_label->setAttributeValue("for", "ste-id-input");
  ste_id_input = new Wt::WLineEdit(ste_options_fg1);
  ste_id_input->setStyleClass("form-control");
  ste_id_input->setId("ste-id-input");

  Wt::WContainerWidget *ste_options_fg2 = new Wt::WContainerWidget(ste_options_modal_table->elementAt(0,1));
  ste_options_fg2->addStyleClass("form-group");
  Wt::WLabel *ste_ss_label = new Wt::WLabel("Symbol Set: ", ste_options_fg2);
  ste_ss_label->setStyleClass("control-label");
  ste_ss_label->setAttributeValue("for", "ste-ss-input");
  ste_ss_input = new Wt::WLineEdit(ste_options_fg2);
  ste_ss_input->setStyleClass("form-control");
  ste_ss_input->setId("ste-ss-input");

  Wt::WContainerWidget *ste_options_fg3 = new Wt::WContainerWidget(ste_options_modal_table->elementAt(1,0));
  ste_options_fg3->addStyleClass("form-group");
  Wt::WLabel *ste_start_label = new Wt::WLabel("Start Type:", ste_options_fg3);
  ste_start_label->setStyleClass("control-label");
  ste_start_label->setAttributeValue("for", "ste-start-select");
  ste_start_select = new Wt::WComboBox(ste_options_fg3);
  ste_start_select->setStyleClass("form-control");
  ste_start_select->setId("ste-start-select");
  ste_start_select->addItem("none");
  ste_start_select->addItem("all-input");
  ste_start_select->addItem("start-of-data");

  Wt::WContainerWidget *ste_options_fg4 = new Wt::WContainerWidget(ste_options_modal_table->elementAt(1,1));
  ste_options_fg4->addStyleClass("form-group");
  reporting_check = new Wt::WCheckBox(" Reporting?", ste_options_fg4);
  Wt::WBreak *ste_line_break = new Wt::WBreak(ste_options_fg4);
  ste_rep_label = new Wt::WLabel("Report Code: ", ste_options_fg4);
  ste_start_label->setStyleClass("control-label");
  ste_start_label->setAttributeValue("for", "ste-rep-input");
  ste_rep_input = new Wt::WLineEdit(ste_options_fg4);
  ste_rep_input->setStyleClass("form-control");
  ste_rep_input->setId("ste-rep-input");

  ste_rep_label->hide();
  ste_rep_input->hide();
  reporting_check->clicked().connect(std::bind( [=] () {
	if (reporting_check->isChecked()) {
	  ste_rep_label->show();
	  ste_rep_input->show();
	}
	else {
	  ste_rep_label->hide();
	  ste_rep_input->hide();
	}
      }));
  Wt::WText *ste_error_msg = new Wt::WText(ste_options_modal_body);
  ste_error_msg->setInline(false);
  ste_error_msg->setStyleClass("alert alert-danger");
  ste_error_msg->hide();

  Wt::WContainerWidget *ste_options_modal_footer = new Wt::WContainerWidget(ste_options_modal_content);
  ste_options_modal_footer->setStyleClass("modal-footer");
  Wt::WPushButton *ste_options_close_btn = new Wt::WPushButton("Cancel", ste_options_modal_footer);
  ste_options_close_btn->setStyleClass("btn btn-default");
  ste_options_close_btn->setAttributeValue("data-dismiss", "modal");
  ste_options_close_btn->clicked().connect(std::bind( [=] () {
	// Reset modal
	ste_id_input->setText("");
	ste_ss_input->setText("");
	ste_rep_input->setText("");
	ste_rep_input->hide();
	ste_rep_label->hide();
	ste_start_select->setCurrentIndex(0);
	reporting_check->setCheckState(Wt::CheckState::Unchecked);	 
	ste_error_msg->hide();
      }));
  Wt::WValidator *validator = new Wt::WValidator(true);
  ste_id_input->setValidator(validator);
  ste_ss_input->setValidator(validator);
  ste_options_create_btn = new Wt::WPushButton("Create STE", ste_options_modal_footer);
  ste_options_create_btn->setStyleClass("btn btn-primary"); 
  ste_options_create_btn->setId("create-ste-btn");
  ste_options_create_btn->clicked().connect(std::bind( [=] () {
	// Input validation - Id and Symbol Set are mandatory
	if (ste_id_input->validate() == Wt::WValidator::Valid 
	    && ste_ss_input->validate() == Wt::WValidator::Valid) {
	  auto elements = ap.getElements();
	  // Does element with this ID already exist?
	  if (ste_id_input->text().toUTF8() != ste_id && elements.find(ste_id_input->text().toUTF8()) != elements.end()) { // yes
	    ste_error_msg->setText("ERROR: An STE with this ID already exists.");
	    ste_error_msg->show();
	  } 
	  else { // Unique ID, good to go!
	    std::string id = ste_id_input->text().narrow();
	    std::string ss = ste_ss_input->text().narrow();
	    std::string rep = (reporting_check->isChecked()) ? ("Report Code: " + ste_rep_input->text().narrow()) : "";
	    std::string start = (ste_start_select->currentIndex() > 0) ? ("Start Type: " +  ste_start_select->currentText().narrow()) : "";
	    std::string type = "node";

	    // If start
	    if (start.length() > 0)
	      type = "start";
	    // If reporting
	    else if (rep.length() > 0)
	      type = "report";

	    if (editingSTE) { // If editing existing STE, update automata reference and graph visual
	      STE * element = static_cast <STE *>(ap.getElements()[ste_id]);

	      if (ste_start_select->currentText().narrow() == "none") {
		// If start type is none, remove from the start STEs vector
		auto starts = ap.getStarts();
		for (auto it = starts.begin(); it != starts.end(); ++it) {
		  if ((*it)->getId() == ste_id) {
		    starts.erase(it);
		    break;
		  }
		}
		starts.erase(std::remove(starts.begin(), starts.end(), element), starts.end());
		std::cout << "Removed start STE" << std::endl;
	      }
	      
	      // Set all of the element's parameters
	      element->setId(id);
	      element->setReporting(rep.length() > 0);
	      element->setReportCode(ste_rep_input->text().toUTF8());
	      element->setSymbolSet(ss);
	      element->setStart(ste_start_select->currentText().narrow());

	      // Unescape all escaped characters in the symbol set for Javascript
	      int start_pos = 0;
	      while ((start_pos = ss.find(R"(\)", start_pos)) != std::string::npos) {
		ss.replace(start_pos, 1, R"(\\)");
		start_pos += 2;
	      }	      

	      // Send data to update Javascript
	      doJavaScript("updateSTEData('" + id + "','" + ss + "','" + start + "','" + rep + "','" + type + "')");
	    }
	    else {// Add the STE to the automata and the node to the graph

	      STE *newSTE = new STE(id, ss, ste_start_select->currentText().toUTF8());
	      if (type == "report") {
		newSTE->setReporting(true);
		newSTE->setReportCode(ste_rep_input->text().toUTF8());
	      }	  
	      ap.rawAddSTE(newSTE);

	      // Unescape all escaped characters in the symbol set for Javascript
	      int start_pos = 0;
	      while ((start_pos = ss.find(R"(\)", start_pos)) != std::string::npos) {
		ss.replace(start_pos, 1, R"(\\)");
		start_pos += 2;
	      }	      

	      // Send data to update Javascript
	      doJavaScript("addSTE('" + id + "','" + ss + "','" + rep + "','" + start + "','" + type + "')");
	    }

	    // Reset modal
	    ste_id_input->setText("");
	    ste_ss_input->setText("");
	    ste_rep_input->setText("");
	    ste_rep_input->hide();
	    ste_rep_label->hide();
	    ste_start_select->setCurrentIndex(0);
	    reporting_check->setCheckState(Wt::CheckState::Unchecked);	 
	    ste_error_msg->hide();
	    ste_options_title->setText("<h3 class='modal-title'>Create STE</h3>");
	    ste_options_create_btn->setText("Create STE"); 
	    editingSTE = false;
	  }
	}
	else {
	    ste_error_msg->setText("ERROR: ID and Symbol Set are required fields.");
	    ste_error_msg->show();
	}
      }));


  // ************** //
  // *** FOOTER *** //
  // ************** //


  Wt::WContainerWidget *footer = new Wt::WContainerWidget(full_page_container);
  footer->setStyleClass("footer");

  /* Character stream table */

  Wt::WContainerWidget *input_display = new Wt::WContainerWidget(footer);
  input_display->setStyleClass("input-display");
  input_display->setId("input-display-container");
  input_display_table = new Wt::WTable(input_display);
  input_display_table->setId("input-table");
  input_display_table->setStyleClass("input-display-table");

  /* Navigation Bar Tabs */

  std::string navbar_string = 
    std::string("<ul class='nav nav-tabs'>") +
    "<li role='presentation' class='active' id='graph-tab'>" +
    "<a role='button' onClick='graph_tab_clicked()'>Graph Settings" +
    "</a></li>" +
    "<li role='presentation' id='sim-tab' style='display: none'>" +
    "<a role='button' onClick='sim_tab_clicked()'>Simulation Tools" +
    "</a></li>" +
    "<li role='presentation' id='editor-tab' style='display: none'>" +
    "<a role='button' onClick='editor_tab_clicked()'>Editor Tools" +
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
  Wt::WTable *footer_table = new Wt::WTable(graph_settings);
  footer_table->setHeaderCount(1);
  footer_table->setStyleClass("footer-table");

  /* Table Headers */

  footer_table->elementAt(0,0)->addWidget(new Wt::WText("Node Size: "));
  footer_table->elementAt(0,1)->addWidget(new Wt::WText("Edge Thickness: "));
  footer_table->elementAt(0,2)->addWidget(new Wt::WText("Width/Height Ratio: "));
  footer_table->elementAt(0,3)->addWidget(new Wt::WText("Rotation: "));
  Wt::WCheckBox *disable_edges = new Wt::WCheckBox(" Render Edges?", footer_table->elementAt(0,4));
  disable_edges->setId("render-edge-box");
  disable_edges->setChecked(true);
  disable_edges->setAttributeValue("onclick", "toggleEdges()");
  Wt::WPushButton *reset_camera_btn = new Wt::WPushButton("Reset Camera", footer_table->elementAt(0,5));
  reset_camera_btn->setStyleClass("btn btn-primary");
  reset_camera_btn->setId("reset-camera-btn");

  /* Table Footers/Inputs */

  Wt::WText *node_slider = new Wt::WText("<input type='range' min='0' max='57' step='1' value='25' id='node-slider'>", Wt::XHTMLUnsafeText, footer_table->elementAt(1,0));
  Wt::WText *edge_slider = new Wt::WText("<input type='range' min='0' max='40' step='1' value='13' id='edge-slider'>", Wt::XHTMLUnsafeText, footer_table->elementAt(1,1));
  Wt::WText *width_slider = new Wt::WText("<input type='range' min='0' max='55' step='1' value='12' id='width-slider'>", Wt::XHTMLUnsafeText, footer_table->elementAt(1,2));
  Wt::WText *angle_slider = new Wt::WText("<input type='range' min='-180' max='180' step='15' value='0' id='angle-slider'>", Wt::XHTMLUnsafeText, footer_table->elementAt(1,3));
  Wt::WCheckBox *heat_mode_check = new Wt::WCheckBox(" Heat Map Mode", footer_table->elementAt(1,4));
  heat_mode_check->setId("heat-mode-box");
  heat_mode_check->setChecked(false);
  Wt::WAnchor *graph_legend_btn = new Wt::WAnchor("graphLegend.html", "View Graph Legend", footer_table->elementAt(1,5));
  graph_legend_btn->setStyleClass("btn btn-default");
  graph_legend_btn->setId("graph-legend-btn");
  graph_legend_btn->setAttributeValue("onClick", "javascript:window.open(\"graphLegend.html\", \"Graph Legend\", \"menubar=0,resizable=0,width=350,height=350\"); return false;");


  // ************************ //
  // *** SIMULATION TOOLS *** //
  // ************************ //
  
  Wt::WContainerWidget *simulation_tools = new Wt::WContainerWidget(footer);
  simulation_tools->setId("sim-tools");
  simulation_tools->setStyleClass("settings-container");
  simulation_tools->hide();
  Wt::WTable *sim_tools_table = new Wt::WTable(simulation_tools);
  sim_tools_table->setHeaderCount(1);
  sim_tools_table->setStyleClass("footer-table");

  /* Table Headers */

  sim_tools_table->elementAt(0,0)->addWidget(new Wt::WText("Cycle Index: "));
  sim_tools_table->elementAt(0,1)->addWidget(new Wt::WText("Simulation Controls: "));
  sim_tools_table->elementAt(0,1)->setColumnSpan(3);
  Wt::WText *play_speed_text = new Wt::WText("Play Speed: Fastest", sim_tools_table->elementAt(0,4));
  play_speed_text->setId("play-speed-text");
  sim_tools_table->elementAt(0,4)->setWidth(200);

  /* Table Content/Inputs */

  Wt::WText *cycle_index = new Wt::WText("0", sim_tools_table->elementAt(1,0));
  cycle_index->setId("cycle-index-text");
  cycle_index->setStyleClass("cycle-counter");
  sim_rev = new Wt::WPushButton("<span class='glyphicon glyphicon-step-backward' aria-hidden='true'></span> Step", sim_tools_table->elementAt(1,1));
  sim_rev->setTextFormat(Wt::XHTMLUnsafeText);
  sim_rev->setStyleClass("btn btn-primary");
  sim_rev->setId("sim-rev-btn");  
  sim_step = new Wt::WPushButton("Step <span class='glyphicon glyphicon-step-forward' aria-hidden='true'></span>", sim_tools_table->elementAt(1,2));
  sim_step->setTextFormat(Wt::XHTMLUnsafeText);
  sim_step->setStyleClass("btn btn-primary");
  sim_step->setId("sim-step-btn");
  Wt::WPushButton *play_btn = new Wt::WPushButton("Play Simulation <span class='glyphicon glyphicon-play' aria-hidden='true'></span>", sim_tools_table->elementAt(1,3));
  play_btn->setTextFormat(Wt::XHTMLUnsafeText);
  play_btn->setStyleClass("btn btn-primary");
  play_btn->setId("play-sim-btn");
  Wt::WText *speed_slider = new Wt::WText("<input type='range' min='-2' max='0' step='.1' value='0' id='speed-slider'>", Wt::XHTMLUnsafeText, sim_tools_table->elementAt(1,4));
  Wt::WCheckBox *hex_mode_check = new Wt::WCheckBox(" Hex Char Mode", sim_tools_table->elementAt(1,5));
  hex_mode_check->setId("hex-mode-box");
  hex_mode_check->setChecked(false);
  Wt::WCheckBox *stop_sim_report_check = new Wt::WCheckBox(" Stop on Report", sim_tools_table->elementAt(0,5));
  stop_sim_report_check->setId("stop-sim-report-box");
  stop_sim_report_check->setChecked(false);
  Wt::WAnchor *download_reports = new Wt::WAnchor(sim_tools_table->elementAt(1,6));
  download_reports->setAttributeValue("role", "button");
  download_reports->setAttributeValue("download", "reports.txt");		    
  download_reports->setId("dl-rep-btn");
  download_reports->setStyleClass("btn btn-default");
  download_reports->setText("Download Report Record <span class='glyphicon glyphicon-save' aria-hidden='true'></span>");
  download_reports->setTextFormat(Wt::XHTMLUnsafeText);
  
  /* Hidden Simulation Tools */
  // For Javascript usage only

  Wt::WPushButton *hidden_play_btn = new Wt::WPushButton(simulation_tools);
  hidden_play_btn->setId("hidden-play-btn");
  hidden_play_btn->hide();
  Wt::WPushButton *hidden_cache_btn = new Wt::WPushButton(simulation_tools);
  hidden_cache_btn->setId("hidden-cache-btn");
  hidden_cache_btn->hide();
  hidden_cache_btn->clicked().connect( std::bind( [=] () {
	  addToJSCache(cache_fetch_size);
      }));
  

  // ******************** //
  // *** EDITOR TOOLS *** //
  // ******************** //
  
  Wt::WContainerWidget *editor_tools = new Wt::WContainerWidget(footer);
  editor_tools->setId("editor-tools");
  editor_tools->setStyleClass("settings-container");
  editor_tools->hide();
  Wt::WTable *editor_tools_table = new Wt::WTable(editor_tools);
  editor_tools_table->setHeaderCount(1);
  editor_tools_table->setStyleClass("footer-table");

  /* Table Headers */
  
  editor_tools_table->elementAt(0,0)->addWidget(new Wt::WText("Selector Tools"));
  editor_tools_table->elementAt(0,2)->addWidget(new Wt::WText("Select All"));
  editor_tools_table->elementAt(0,3)->addWidget(new Wt::WText("Unselect All"));
  editor_tools_table->elementAt(0,4)->addWidget(new Wt::WText("Select Neighbors"));
  editor_tools_table->elementAt(0,5)->addWidget(new Wt::WText("Select Multiple"));

  /* Table Inputs */
  Wt::WPushButton *toggle_lasso = new Wt::WPushButton("Lasso Tool", editor_tools_table->elementAt(1,0));
  toggle_lasso->setStyleClass("btn btn-primary");
  toggle_lasso->setId("toggle-lasso-btn");
  Wt::WPushButton *toggle_rect = new Wt::WPushButton("Box Selector", editor_tools_table->elementAt(1,1));
  toggle_rect->setStyleClass("btn btn-primary");
  toggle_rect->setId("toggle-rect-btn");
  Wt::WText *spaceA = new Wt::WText("<kbd>spacebar</kbd> + <kbd>a</kbd>", Wt::XHTMLUnsafeText, editor_tools_table->elementAt(1,2));
  Wt::WText *spaceU = new Wt::WText("<kbd>spacebar</kbd> + <kbd>u</kbd>", Wt::XHTMLUnsafeText, editor_tools_table->elementAt(1,3));
  Wt::WText *spaceE = new Wt::WText("<kbd>spacebar</kbd> + <kbd>e</kbd>", Wt::XHTMLUnsafeText, editor_tools_table->elementAt(1,4));
  Wt::WText *spaceClick = new Wt::WText("<kbd>spacebar</kbd> + Left Click", Wt::XHTMLUnsafeText, editor_tools_table->elementAt(1,5));
  Wt::WPushButton *save_anml_btn = new Wt::WPushButton("Save ANML File", editor_tools_table->elementAt(1,6));
  save_anml_btn->setStyleClass("btn btn-primary");
  save_anml_btn->setId("save-anml-btn");
  save_anml_btn->clicked().connect(std::bind ( [=] () {
	// Export code from VASim
	std::string str = "";

	// xml header
	str += "<anml version=\"1.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";
	str += "<automata-network id=\"vasim\">\n";

	for(auto el : ap.getElements()) {
	  str += el.second->toANML();
	  str += "\n";
	}

	// xml footer
	str += "</automata-network>\n";
	str += "</anml>\n";
	
	// Unescape all escaped newline characters
	int start_pos = 0;
	while ((start_pos = str.find("\n", start_pos)) != std::string::npos) {
	  str.replace(start_pos, 1, "\\n");
	  start_pos += 3;
	}

	doJavaScript("exportFile('automata_file.anml', '" + str + "\')");
      }));
  
  

  /* Graph container */

  Wt::WContainerWidget *container = new Wt::WContainerWidget(full_page_container);
  container->setId("container");
  Wt::WContainerWidget *graph_container = new Wt::WContainerWidget(container);
  graph_container->setId("graph-container");
  

  // *************************** //
  // *** UPLOAD FILE PROCESS *** //
  // *************************** //


  /* Uploading automata file */

  automata_file_upload->changed().connect( std::bind([=] () {
	load_modal_message->setText("Uploading file...");
	std::cout << "STATUS: UPLOADING AUTOMATA FILE" << std::endl;
	newFileUploaded();
	loadInputTable();
	doJavaScript("$('#loading-graph-modal').modal('show');");
	automata_file_upload->upload();
      }));

  /* Converting file and displaying graph */

  automata_file_upload->uploaded().connect(std::bind([=] () {
	std::vector< Wt::Http::UploadedFile > file_vector = automata_file_upload->uploadedFiles();
	std::string filename = automata_file_upload->clientFileName().toUTF8();
	fn = file_vector.data()->spoolFileName();
	automataUploaded = true;

	if (filename.substr(filename.find_last_of(".")) == ".json") { // Handle pre-formatted JSON files
	  std::ifstream ifs(fn);
	  std::string content( (std::istreambuf_iterator<char>(ifs) ),
			       (std::istreambuf_iterator<char>()    ) );
	  std::string json_string = content;
	 load_modal_message->setText("Loading graph...");
	  processEvents();
	  std::cout << "STATUS: LOADING GRAPH IN SIGMA" << std::endl;
	  validAutomata = false;
	  doJavaScript("loadGraph("+json_string+")");
	} else if (filename.substr(filename.find_last_of(".")) == ".mnrl" ||
		 filename.substr(filename.find_last_of(".")) == ".anml") { // Handle MNRL and ANML
	  // Displays option modal
	  doJavaScript("$('#loading-graph-modal').modal('hide');");
	  doJavaScript("$('#options-modal').modal('show');");
	} else { // Unsupported file type
	  doJavaScript("$('#loading-graph-modal').modal('hide');");
	  error_modal_message->setText("Invalid automata file.");
	  doJavaScript("$('#error-modal').modal('show');");
	  validAutomata = false;
	}
	
      }));

  /* Uploading input file */

  input_file_upload->changed().connect( std::bind([=] () {
	load_modal_message->setText("Uploading file...");
	std::cout << "STATUS: UPLOADING INPUT FILE" << std::endl;
	doJavaScript("$('#loading-graph-modal').modal('show')");
	input_file_upload->upload();
      }));

  /* Handle uploaded file */

  input_file_upload->uploaded().connect(std::bind([=] () {
	
	doJavaScript("$('#loading-graph-modal').modal('hide');");
	// Read file contents into global string 'input_string'
	std::vector< Wt::Http::UploadedFile > file_vector = input_file_upload->uploadedFiles();
	loadTextFromFile(""+file_vector.data()->spoolFileName());

      }));

  /* Load DOM elements on page before scripts run */

  processEvents();
  doJavaScript("pageLoad()");

  // Load GET queries in following format:
  // ?a=AutomataName&o=dd&fi=n&fo=m where dd is bool value of global and or optimizations and n and m are numbers
  // ?a=Brill&o=10&fo=5 means load Brill with global opt and fan-out limit of 5

  auto paramMap = env.getParameterMap();
  if (paramMap.find("a") != paramMap.end()) {
    // Load graph with name given at a=,
    // bring up optimization menu only if no opt code is provided
    bool optQ = paramMap.find("o") != paramMap.end() || paramMap.find("fi") != paramMap.end() || paramMap.find("fo") != paramMap.end();
    loadDemoGraph(paramMap["a"][0], !optQ);
    if (optQ) {
      std::string optCode;
      int fi = 0, fo = 0;
      if (paramMap.find("o") != paramMap.end())
	optCode = paramMap["o"][0];
      if (paramMap.find("fi") != paramMap.end())
	fi = std::stoi(paramMap["fi"][0]);
      if (paramMap.find("fo") != paramMap.end())
	fo = std::stoi(paramMap["fo"][0]);
      handleAutomataFile(optCode[0] == '1', optCode[1] == '1', fn, fi, fo);
    }
  }

}

void VASimViz::toggleConnection(std::string sourceId, std::string targetId, bool add) {
  auto elements = ap.getElements();
  if (add)
    ap.addEdge(elements[sourceId], elements[targetId]);
  else
    ap.removeEdge(elements[sourceId], elements[targetId]);
}

void VASimViz::changeSTEData(std::string id, std::string ss, std::string start, std::string rep) {
  ste_id_input->setText(id);
  ste_ss_input->setText(ss);
  ste_start_select->setCurrentIndex(ste_start_select->findText(start));
  ste_rep_input->setText(rep);
  ste_id = id;

  ste_options_title->setText("<h3 class='modal-title'>Update STE Data</h3>");
  if (rep.length() > 0) {
    reporting_check->setChecked(true);
    ste_rep_input->show();
    ste_rep_label->show();
  }
  else {
    reporting_check->setChecked(false);
    ste_rep_input->hide();
    ste_rep_label->hide();
  }
  ste_options_create_btn->setText("Update STE");

  editingSTE = true;

  doJavaScript("$('#add-ste-modal').modal('show')");
}

void VASimViz::deleteSTE(std::string id) {
  ste_id = id;
  delete_modal_text->setText("Are you sure you want to delete <b>" + id + "</b>?");
}

/*
 * Resets simulation settings when new automata or input file is uploaded
 */
void VASimViz::newFileUploaded() {
  cache_index = -1;
  cache = json11::Json::array{};
  doJavaScript("graph_tab_clicked()");
  doJavaScript("$('#sim-tab').hide()");
  doJavaScript("resetSimulation()");
  simulate->setEnabled(true);
}

/*
 * Loads a text file from a file name into 'input_string'
 */
void VASimViz::loadTextFromFile(std::string fn) {
  std::cout << "STATUS: READING FILE AND GENERATING TABLE" << std::endl;
  input_string = "";
  std::ifstream myfile (fn);
  std::string line;
  while ( getline (myfile, line) ){
    input_string += line;
  }
  myfile.close();
  std::cout << "Read file correctly " << std::endl;
  loadInputTable();
}

/*
 * Loads contents of 'input_string' into character stream in plain text and hex
 * For plain text, when character is not printable, displays hex value instead
 */
void VASimViz::loadInputTable() {
  newFileUploaded();
  std::cout << "STATUS: LOADING INPUT TABLE" << std::endl;
    // Add up to 'max_input_display_size' table cells total, each containing a character
  int length = (input_string.length() > max_input_display_size) ? max_input_display_size : input_string.length();
  if (length == 0) return; // Does not load empty table
  input_display_table->clear();
  for (int i = 0; i < length; i++) {
    std::string ch;
    // Load hex stream at row 1
    std::stringstream ss;
    ss << "x\\" << std::hex << std::setw(2) << std::setfill('0') << ((unsigned int)(unsigned char)input_string[i]);
    ch = ss.str();
    input_display_table->elementAt(1,i)->addWidget(new Wt::WText(ch, Wt::PlainText));
    // Load plain text stream at row 0
    if ((uint32_t)input_string[i] >= 20 && (uint32_t)input_string[i] <= 126)
      ch = std::string(1, input_string[i]);
    else {
      std::stringstream ss;
      ss << "x\\" << std::hex << std::setw(2) << std::setfill('0') << ((unsigned int)(unsigned char)input_string[i]);
      ch = ss.str();
    }
    
    input_display_table->elementAt(0,i)->addWidget(new Wt::WText(ch, Wt::PlainText));
  }

  doJavaScript("setInputLength(" + std::to_string(input_string.length()) + ")");
  input_display_table->rowAt(1)->hide();

  validInput = true;
}

/*
 * Initializes global automata object with user-defined settings
 * Loads automata as graph via Javascript to Sigma.js
 */
void VASimViz::handleAutomataFile(bool global,  bool OR, std::string fn, int32_t fanin_limit, int32_t fanout_limit) 
{
  // Create Automata object from file
  std::cout << "STATUS: CREATING AUTOMATA OBJECT FROM FILE" << std::endl;
  load_modal_message->setText("Creating Automata object from file...");
  doJavaScript("$('#loading-graph-modal').modal('show')");
  processEvents();

  doJavaScript(std::string("appendOptimizations('") + ((global || OR) ? (std::string("&o=") + ((global) ? "1" : "0") + ((OR) ? "1" : "0")) : "") + 
	       ((fanin_limit > 0) ? ("&fi=" + std::to_string(fanin_limit)) : "") + 
	       ((fanout_limit > 0) ? ("&fo=" + std::to_string(fanout_limit)) : "") + "')");

  Automata a(fn);
  ap = a;
  validAutomata = true;
  uint32_t automata_size = ap.getElements().size();
  
  load_modal_message->setText("Processing optimizations...");
  processEvents();

  // Left Minimization
  if (global) {
    ap.leftMinimize();        
    while(automata_size != ap.getElements().size()) {
      automata_size = ap.getElements().size();
      ap.leftMinimize();
    }
  }

  // Remove OR gates, which are just syntactic sugar (benefitial for some hardware)
  if(OR)
    ap.removeOrGates();

  // Enforce fan-in limit
  if(fanin_limit > 0)
    ap.enforceFanIn(fanin_limit);

  // Enforce fan-out limit
  if(fanout_limit > 0)
    ap.enforceFanOut(fanout_limit);
  
  // Enable profiling to allow simulation data to be collected
  ap.enableProfile();

  // Convert to JSON
  load_modal_message->setText("Converting to JSON format...");
  processEvents();
  std::cout << "STATUS: CONVERTING AUTOMATA TO JSON" << std::endl;
  std::string json_string = SigmaJSONWriter::writeToJSON(&ap);

  // Load JSON in graph
  load_modal_message->setText("Loading graph...");
  processEvents();
  std::cout << "STATUS: LOADING GRAPH IN SIGMA" << std::endl;
  doJavaScript("loadGraph("+json_string+")");
  
}

/*
 * Creates 'random' tree automata; for use in layout algorithm debugging
 */
void VASimViz::loadRandomAutomata() {
  ap = RandomAutomata::generateRandomAutomata();
  ap.enableProfile();
  std::string json_string = SigmaJSONWriter::writeToJSON(&ap);
  doJavaScript("loadGraph(" + json_string + ")");
}

/*
 * Simulates a single step of the automata on a character
 * Returns JSON data for Sigma to update graph coloring
 */
std::string VASimViz::simulateAutomata(char symbol) {
  return SigmaJSONWriter::simulateStep(&ap, symbol);
}

/*
 * Initializes simlation and loads first graph from cache
 */
void VASimViz::beginSimulation() {
  ap.initializeSimulation();
  input_display_table->elementAt(0,0)->setStyleClass("highlight");
  input_display_table->elementAt(1,0)->setStyleClass("highlight");
  doJavaScript("$('#loading-graph-modal').modal('show')");
  load_modal_message->setText("Simulating automata into cache...");
  processEvents();
  addToJSCache(cache_fetch_size + 1);
  doJavaScript("$('#loading-graph-modal').modal('hide')");
  processEvents();
  doJavaScript("stepFromCache(0)");
}

/*
 * Simulates and stores new graphs then sends to JavaScript
 */
void VASimViz::addToJSCache(int numGraphs) {
  // 'graph_vector' contains all newly fetched graphs/data
  std::vector<json11::Json> graph_vector;
  std::string err;
  int prevIndex = cache_index;
  // Set cache_index to the index of last simulated graph
  cache_index = (cache_index + numGraphs < input_string.length() - 1) ? cache_index + numGraphs : input_string.length() - 1;
  std::cout << "STATUS: Caching " << cache_index - prevIndex << " new graphs from index " << prevIndex + 1 << " to index " << cache_index << std::endl;
  // Load last n elements of current cache where n = cache_fetch_size/2
  for (int i = cache[0].array_items().size() - cache_fetch_size + (cache_index - prevIndex) - cache_fetch_size/2; i < cache[0].array_items().size(); i++) {
    graph_vector.push_back(cache[0].array_items()[i]);
  }
  std::cout << "\tLoaded " << graph_vector.size() << " previous items from current cache to new cache" << std::endl;
  // Loop from last cached graph index + 1 to new cached index
  for (int i = prevIndex + 1; i <= cache_index; i++) {
    graph_vector.push_back(json11::Json::parse(simulateAutomata(input_string[i]), err));
  }
  cache = json11::Json::array {graph_vector};
  //std::cout << cache.dump() << std::endl;
  doJavaScript("setCachedGraphs(" + std::to_string(cache_index) + ", " + cache.dump() + ")");
  std::cout << "\tCache length is now " << graph_vector.size() << " graphs." << std::endl; 

}

/*
 * Loads graph from ANMLZoo library with its corresponding input
 * When multiple automata/inputs are provided, the first lexicographically is loaded
 */
void VASimViz::loadDemoGraph(std::string name, bool userLoaded) {
  std::cout << "STATUS: LOADING GRAPH '" << name << "' FROM ANMLZOO" << std::endl;
  // Allows for simulation button to be clicked
  automataUploaded = true;
  validAutomata = true;
  validInput = true;

  newFileUploaded(); 

  if (name == "Brill") {
    loadTextFromFile("ANMLZoo/Brill/inputs/brill_1MB.input");
    fn = "ANMLZoo/Brill/anml/brill.1chip.anml";
    anmlzoo_combo->setCurrentIndex(1);
  }
  else if (name == "ClamAV") {
    loadTextFromFile("ANMLZoo/ClamAV/inputs/vasim_1MB.input");
    fn = "ANMLZoo/ClamAV/anml/515_nocounter.1chip.anml";
    anmlzoo_combo->setCurrentIndex(2);
  }
  else if (name == "Dotstar") {
    loadTextFromFile("ANMLZoo/Dotstar/inputs/backdoor_1MB.input");
    fn = "ANMLZoo/Dotstar/anml/backdoor_dotstar.1chip.anml";
    anmlzoo_combo->setCurrentIndex(3);
  }
  else if (name == "EntityResolution") {
    loadTextFromFile("ANMLZoo/EntityResolution/inputs/1000_1MB.input");
    fn = "ANMLZoo/EntityResolution/anml/1000.1chip.anml";
    anmlzoo_combo->setCurrentIndex(4);
  }
  else if (name == "Fermi") {
    loadTextFromFile("ANMLZoo/Fermi/inputs/rp_input_1MB.input");
    fn = "ANMLZoo/Fermi/anml/fermi_2400.1chip.anml";
    anmlzoo_combo->setCurrentIndex(5);
  }
  else if (name == "Hamming") {
    loadTextFromFile("ANMLZoo/Hamming/inputs/hamming_1MB.input");
    fn = "ANMLZoo/Hamming/anml/93_20X3.1chip.anml";
    anmlzoo_combo->setCurrentIndex(6);
  }
  else if (name == "Levenshtein") {
    loadTextFromFile("ANMLZoo/Levenshtein/inputs/DNA_1MB.input");
    fn = "ANMLZoo/Levenshtein/anml/24_20x3.1chip.anml";
    anmlzoo_combo->setCurrentIndex(7);
  }
  else if (name == "PowerEN") {
    loadTextFromFile("ANMLZoo/PowerEN/inputs/poweren_1MB.input");
    fn = "ANMLZoo/PowerEN/anml/complx_01000_00123.1chip.anml";
    anmlzoo_combo->setCurrentIndex(8);
  }
  else if (name == "Protomata") {
    loadTextFromFile("ANMLZoo/Protomata/inputs/uniprot_fasta_1MB.input");
    fn = "ANMLZoo/Protomata/anml/2340sigs.1chip.anml";
    anmlzoo_combo->setCurrentIndex(9);
  }
  else if (name == "RandomForest") {
    loadTextFromFile("ANMLZoo/RandomForest/inputs/mnist_1MB.input");
    fn = "ANMLZoo/RandomForest/anml/300f_15t_tree_from_model_MNIST.anml";
    anmlzoo_combo->setCurrentIndex(10);
  }
  else if (name == "Snort") {
    loadTextFromFile("ANMLZoo/Snort/inputs/snort_1MB.input");
    fn = "ANMLZoo/Snort/anml/snort.1chip.anml";
    anmlzoo_combo->setCurrentIndex(11);
  }
  else if (name == "SPM") {
    loadTextFromFile("ANMLZoo/SPM/inputs/SPM_1MB.input");
    fn = "ANMLZoo/SPM/anml/bible_size4.1chip.anml";
    anmlzoo_combo->setCurrentIndex(12);
  }
  else if (name == "Synthetic") {
    loadTextFromFile("ANMLZoo/Synthetic/inputs/1MB.input");
    fn = "ANMLZoo/Synthetic/anml/BlockRings.anml";
    anmlzoo_combo->setCurrentIndex(13);
  }
  else if (name == "Donut") {
    loadTextFromFile("Donut/donut.input");
    fn = "Donut/donut.anml";
    anmlzoo_combo->setCurrentIndex(14);
  }
  else if (name == "Random") {
    loadRandomAutomata();
    userLoaded = false;
    anmlzoo_combo->setCurrentIndex(15);
  }
  else
    return;

  doJavaScript("window.history.pushState({}, 'Title', '/?a=" + name + "')");
  if (userLoaded) {
    doJavaScript("$('#loading-graph-modal').modal('hide');");
    doJavaScript("$('#options-modal').modal('show');");
  }
}


Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new VASimViz(env);
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
