'use strict';

// *********************** //
// *** SIGMA FUNCTIONS *** //
// *********************** //

/** 
 * Finds all descendants of a given node
 * Modified function from Sigma.js
 * @param {string} nodeId - the string ID of the parent node
 * @param {Object} connectedNodes - the array containing the references of nodes added recursively
 * @returns {array} all nodes that are descendants of the parent node
 */
sigma.classes.graph.addMethod('children', function(nodeId, connectedNodes) {

    var k,
    index = this.outNeighborsIndex[nodeId] || {};
    connectedNodes[nodeId] = sig.graph.nodes(nodeId);

    for (k in index) {
	if (!connectedNodes[k])
	    connectedNodes = sig.graph.children(k, connectedNodes);
    }
    return connectedNodes;    

});

/**
 * Finds all outgoing edges of a given node
 * Modified graph neighbor function from Sigma.js
 * @param {string} nodeId - the ID of the parent node
 * @returns {Object} all outgoing edges from parent node
*/
sigma.classes.graph.addMethod('outEdges', function(nodeId) {
    var k,
    e,
    outgoingEdges = {},
    index = this.outNeighborsIndex[nodeId] || {};

    for (k in index) {
	for (e in this.outNeighborsIndex[nodeId][k]) {
            outgoingEdges[e] = this.edgesIndex[e];
	}
    }
    return outgoingEdges;
});

/**
 * Changes basic data of a node
 * If the node's ID has changed, creates a new node
 * then connects all edges to new node before deleting old one
 * @param {string} nodeId - the ID of the node being changed
 * @param {Object} data - the object containing the new id, symbol set, report code, start type, and type
 */
sigma.classes.graph.addMethod('changeData', function(nodeId, data) {
    // Find reference to the original node
    var node = sig.graph.nodes(nodeId),
    // Construct a new node object
    newNode = {
	id: data.id,
	data: {
	    ss: data.ss,
	    rep_code: data.rep_code,
	    start: data.start,
	    type: data.type
	},
	color: getColor(data.type),
	originalColor: getColor(data.type),
	x: node.x,
	y: node.y,
	size: node.size
    },
    in_index = this.inNeighborsIndex[nodeId] || {},
    out_index = this.outNeighborsIndex[nodeId] || {},
    k,
    e;

    automataChanged = true;

    if (nodeId == data.id) { // Node ID hasn't changed; update data
	node.data = newNode.data;
	node.color = newNode.color;
	return;
    }
    // Add new node
    sig.graph.addNode(newNode);
    
    // Transferring all edges
    // Edges are dropped so edge IDs can be preserved

    // For all nodes with edges pointing to newNode
    for (k in in_index)
	for (e in in_index[k]) {
	    // Remove edge and add new one to newNode
	    sig.graph.dropEdge(e);

	    sig.graph.addEdge({
		id: e,
		source: k,
		target: newNode.id,
		size: edgeSize * elemScalar
	    });
	}
    // For all nodes with edges pointing from newNode
    for (k in out_index)
	for (e in out_index[k]) {
	    // Remove edge and add new one from newNode
	    sig.graph.dropEdge(e);
	    sig.graph.addEdge({
		id: e,
		source: newNode.id,
		target: k,
		size: edgeSize * elemScalar
	    });
	}

    sig.graph.dropNode(nodeId);
    
    sig.refresh({skipIndexation: false});
});

// ************************ //
// *** GLOBAL VARIABLES *** //
// ************************ //

// Default settings
var page_settings = new Settings().settings();
var sig_settings;
var tooltip_def_config;
var tooltip_edit_config;

// Sigma variables
var sig = new sigma();
var changedGraph = {nodes: [], edges: []};
var cachedGraphs;
var locate_plugin;
var tooltips;
var activeState;
var dragListener;
var select;
var keyboard;
var lasso;
var rectSelect;
var heatmap;
var editNodeId;
var bounds;
var automataChanged = false;

// Graph settings variable defaults
var heatMode = false;
var draw_edges = true;
var nodeSize = 1;
var edgeSize = .1;
var graphWidth = 12;
var graphHeight = 10;
var editor_mode = false;
var elemScalar = 1;
var deleteEdgeMode = false;
var del_index = 0;
var del_id = "";

// Simulation variables
var simIndex = 0;
var cache_length = 0;
var cache_index = -1;
var cache_fetch_size = 100;
var playSim = false;
var speed = 0;
var updatingCache = false;
var input_length = 0;
var stopSimOnReport = false;
var reportRecord = "";
var textFile = null;
var stopSimOnCycle = -1;

// JQuery DOM elements
var $download_rep_btn; 
var $play_sim_btn;
var $sim_step_btn;
var $sim_rev_btn;
var $hidden_play_btn;
var $input_display_container;
var $table;

// Other DOM elements
var body;
var graph_container;


// ************************ //
// *** HELPER FUNCTIONS *** //
// ************************ //

/**
 * Appends the argument to the end of the current page URL
 * @param {string} opt - the opt code to be appended to the URL
 */
function appendOptimizations(opt) {
    if (~window.location.href.indexOf("?a=")) {
	var path = window.location.href.substring(window.location.href.indexOf("?a="));
	window.history.pushState({}, 'Title', path + opt);
    }
}

/**
 * Saves a file to the user's computer
 * @param {string} fn - the name of the file to be saved
 * @param {string} data - the contents of the file to be saved
*/
function exportFile(fn, data) {
    var blob = new Blob([data], {type: "text/plain"});
    saveAs(blob, fn);
}

/**
 * Gets the corresponding color based on the input data type
 * @param {string} data - the 'type' or 'activity' parameter of a node
 * @returns {string} the corresponding color code as listed in global settings
 */
function getColor(data) {
    switch (data) {
	// Inactive -- data = 'type' parameter
    case "node":
	return page_settings["defaultSTEColor"];
    case "start":
	return page_settings["defaultStartSTEColor"];
    case "report":
	return page_settings["defaultReportSTEColor"];
	// Activity -- data = 'activity' parameter
    case "enabled":
	return page_settings["enabledSTEColor"];
    case "activated":
	return page_settings["activatedSTEColor"];
    case "reporting":
	return page_settings["reportingSTEColor"];
    default:
	return page_settings["defaultSpecialSTEColor"];
    }
}

/**
 * Changes the simulation index when the character stream is clicked
 * @param {Number} index - the index to switch to
 */
function indexClick(index) {
    // Disable/enable sim buttons
    if (index == cache_index) { // end of cache; disable moving forward 
	$sim_step_btn.prop("disabled", true);
	$play_sim_btn.prop("disabled", true);
    }
    else { // otherwise enable
	$sim_step_btn.prop("disabled", false);
	$play_sim_btn.prop("disabled", false);
    }
    if (index == cache_index - cache_length + 1) // beginning of cache; disable moving back
	$sim_rev_btn.prop("disabled", true);
    else // otherwise enable
	$sim_rev_btn.prop("disabled", false);

    stepFromCache(index - simIndex);
}

/**
 * Loads the sigma instance with a new graph and applies default settings
 * @param {Object} json_object - the data structure containing the graph data
 */
function loadGraph(json_object){

    // Set all sliders back to their default
    $('#node-slider').val(page_settings['defaultNodeSize']);
    $('#edge-slider').val(page_settings['defaultEdgeSize']);
    $('#width-slider').val(page_settings['defaultWidthRatio']);
    $('#angle-slider').val(0);

    nodeSizeChange(page_settings['defaultNodeSize']);    
    edgeSizeChange(page_settings['defaultEdgeSize']);
    widthChange(page_settings['defaultWidthRatio']);
    rotationChange(0);
    
    $download_rep_btn.hide();

    sig.graph.clear();
    sig.refresh();

    sig.graph.read(json_object);

    // Set sizes and save color of nodes
    sig.graph.nodes().forEach(function(n) {
	n.size = elemScalar * nodeSize;
	n.color = getColor(n.data.type);
	n.originalColor = n.color;
	n.x = n.x * Math.pow(1.1, graphWidth);
	n.y = ~~n.y;
	n.count = 0;
	if (heatMode)
	    n.color = heatmap.getHexColor(0, true);
    });
    
    // Set size of edges
    sig.graph.edges().forEach(function(e) {
	e.size = elemScalar * edgeSize;
    });

    sig.refresh();
    $('#loading-graph-modal').modal('hide');
}

/**
 * Called on page load, initializes DOM references and settings
 * Adds actions listeners to all input objects
 * Initializes Sigma instance with invisible nodes
 */
function pageLoad() {

    // Set all sliders to their default value
    $('#node-slider').val(page_settings['defaultNodeSize']);
    $('#edge-slider').val(page_settings['defaultEdgeSize']);
    $('#width-slider').val(page_settings['defaultWidthRatio']);

    nodeSizeChange(page_settings['defaultNodeSize']);    
    edgeSizeChange(page_settings['defaultEdgeSize']);
    widthChange(page_settings['defaultWidthRatio']);

    $('#stop-cycle-input').hide();

    // Loading the default settings for a sigma instance
    sig_settings = {
	autoRescale: ['nodePosition'],
	skipIndexation: true,
	hideEdgesOnMove: true,
	zoomMin: .00001,
	zoomMax: 2,
	edgeColor: "default",
	drawEdges: draw_edges,
	edgeHoverColor: "default",
	defaultEdgeHoverColor: page_settings['edgeHoverColor'],
	edgeHoverSizeRatio: 1.3,
	edgeHoverExtremities: true,
	defaultEdgeColor: page_settings['defaultEdgeColor'],
	defaultEdgeType: "arrow",
	defaultNodeType: "fast",
	maxNodeSize: nodeSize,
	minNodeSize: 0,
	maxEdgeSize: edgeSize,
	minEdgeSize: 0,	
	nodesPowRatio: 1,
	edgesPowRatio: 1,
	sideMargin: 5,
	borderSize: 2,
	nodeQuadTreeMaxLevel: 6,
	edgeQuadTreeMaxLevel: 6,
	// Plugin stuff	
	mouseEnabled: true,
	touchEnabled: true,
	defaultNodeActiveBorderColor: page_settings['selectedSTEBorderColor'],
	nodeBorderSize: 0,
	nodeActiveBorderSize: 2
    };

    // The default configuration for the tooltip plugin when not in Editor Mode
    tooltip_def_config = {
	node: [ 
	    {
		show: 'rightClickNode',
		cssClass: 'sigma-tooltip',
		position: 'right',
		template:
		'<ul class="custom-menu">' +
		    '<div class="hover-info"><p>ID: {{id}}</p>' +
		    '<p>Symbol Set: {{data.ss}}</p>' +
		    '<p>{{data.start}}</p>' +
		    '<p>{{data.rep_code}}</p></div>' +
		    '<li onClick="toggleChildren(\'{{id}}\')">Show/Hide Children</li>' +
		    '<li onClick="soloChildren(\'{{id}}\')">Solo Children</li>' +
		    '<li onClick="printNodeData(\'{{id}}\')">- Print Node Data -</li>' +
		    '</ul>',
		renderer: function(node, template) {
		    return Mustache.render(template, node);
		}
	    }
	],
	stage: {
	    template:
	    '<ul class="custom-menu">' +
		'<li onClick="openSearchBar()">Search By ID</li>' +
		'<li onClick="showAllNodes()">Show All Nodes</li>' +
		'<li onClick="toggleEditorMode()">Toggle Editor Mode</li>' +
		'<li onClick="resetCamera()">Reset Camera</li>' +
		'</ul>'
	}
    };

    // The default configuration for the tooltip plugin when in Editor Mode
    tooltip_edit_config = {
	node: [ 
	    {
		show: 'rightClickNode',
		cssClass: 'sigma-tooltip',
		position: 'right',
		template:
		'<ul class="custom-menu">' +
		    '<div class="hover-info"><p>ID: {{id}}</p>' +
		    '<p>Symbol Set: {{data.ss}}</p>' +
		    '<p>{{data.start}}</p>' +
		    '<p>{{data.rep_code}}</p></div>' +
		    '<li onClick="addEdge(\'{{id}}\')">Add Outgoing Connection</li>' +
		    '<li onClick="triggerChangeData(\'{{id}}\')">Change Data</li>' +
		    '<li onClick="triggerDeleteSTE(\'{{id}}\')">Delete STE</li>' +
		    '<li onClick="printNodeData(\'{{id}}\')">- Print Node Data -</li>' +
		    '</ul>',
		renderer: function(node, template) {
		    return Mustache.render(template, node);
		}
	    }
	],
	edge: {
	    show: 'rightClickEdge',
	    template: 
	    '<ul class="custom-menu">' +
		'<li onClick="selectConnectedNodes(\'{{id}}\')">Select Connected STEs</li>' +
		'<li onClick="removeEdge(\'{{id}}\')">Remove Edge</li>' +
		'</ul>',
	    renderer: function(edge, template) {
		return Mustache.render(template, edge);
	    }
	},
	stage: {
	    template:
	    '<ul class="custom-menu">' +
		'<li onClick="triggerAddSTE()">Add STE</li>' +		 
		'<li onClick="showAllNodes()">Show All Nodes</li>' +
		'<li onClick="toggleEditorMode()">Toggle Editor Mode</li>' +
		'<li onClick="resetCamera()">Reset Camera</li>' +
		'<li onClick="printAllNodeData()">- Print All Node Data -</li>' +
		'</ul>'
	}
    };

    // Initialize DOM element references
    $download_rep_btn = $('#dl-rep-btn');
    $play_sim_btn = $('#play-sim-btn');
    $sim_step_btn = $('#sim-step-btn');
    $sim_rev_btn = $('#sim-rev-btn');
    $hidden_play_btn = $('#hidden-play-btn');
    $input_display_container = $('#input-display-container');
    $table = $("#input-table");
    
    body = document.body;

    // Set graph container height to fit between header and footer
    graph_container = document.getElementById("graph-container");
    graph_container.style.top = document.getElementById("click-collapse-bar").offsetHeight + "px";
    graph_container.style.height = "calc(100% - " + 
	document.getElementsByClassName("footer")[0].offsetHeight + 
	"px - " + document.getElementById("click-collapse-bar").offsetHeight + "px)";
    graph_container.style.background = page_settings['stageBGColor'];
    
    /* INPUT LISTENERS */

    // Range sliders
    $('input[type=range]').on('input', function() {
	$(this).trigger('change');
    });
    $('#node-slider').change( function() {
	nodeSizeChange($(this).val());
    });
    $('#edge-slider').change( function() {
	edgeSizeChange($(this).val());
    });
    $('#width-slider').change( function() {
	widthChange($(this).val());
    });
    $('#angle-slider').change( function() {
	rotationChange($(this).val());
    });
    $('#speed-slider').change( function() {
	var val = $(this).val();
	speed = val;
	if (playSim) {
	    $play_sim_btn.html("Play Simulation");
	    $sim_step_btn.prop("disabled", false);
	    $sim_rev_btn.prop("disabled", false);
	    clearInterval(interval);
	    playSim = false;
	}
	if (val == 0)
	    $('#play-speed-text').html("Play Speed: Fastest");
	else if (val >= -.3)
	    $('#play-speed-text').html("Play Speed: Fast");
	else if (val >= -.8)
	    $('#play-speed-text').html("Play Speed: Medium");	
	else if (val >= -1.8)
	    $('#play-speed-text').html("Play Speed: Slow");
	else
	    $('#play-speed-text').html("Play Speed: Slowest");
    });

    // Check boxes
    $('#inhex-mode-box').click(function() {
	$table.find('tr').toggle();
    });
    $('#inheat-mode-box').click(function() {
	toggleHeatMap();
    });
    $('#instop-sim-report-box').click(function() {
	stopSimOnReport = document.getElementById('instop-sim-report-box').checked;
    });

    // Combo Boxes
    $('#stop-combo').change(function() {
	if (this.value == "2") { // 'cycle' is selected
	    $('#stop-cycle-input').show();
	    stopSimOnReport = false;
	}
	else if (this.value == "1") { // 'report' is selected
	    $('#stop-cycle-input').hide();
	    stopSimOnReport = true;
	    stopSimOnCycle = -1;
	    $('#stop-cycle-input').val("");
	}
	else { // 'none' is selected
	    $('#stop-cycle-input').hide();
	    stopSimOnReport = false;
	    stopSimOnCycle = -1;
	    $('#stop-cycle-input').val("");
	}
    });

    // Text Input
    $('#stop-cycle-input').change(function() {
	stopSimOnCycle = +(this.value);
    });

    // Buttons
    $('#reset-camera-btn').click(function() {
	resetCamera();
    });
    $('#toggle-lasso-btn').click(function() {
	toggleLasso();
    });
    $('#toggle-rect-btn').click(function() {
	toggleRect();
    });
    $('#save-graph-btn').click(function() {
	saveGraphData();
    });
    $sim_step_btn.click(function() {
	stepFromCache(1).then(function(response) {
	    $sim_rev_btn.prop("disabled", false);
	    $input_display_container.animate({scrollLeft: simIndex % 1000 * 30 - $(window).width()/2}, 300);
	    if (simIndex == cache_index)
		$sim_step_btn.prop("disabled", true);
	}, function(reject) {
	    console.log(reject);
	});
    });
    $sim_rev_btn.prop("disabled", true);
    $sim_rev_btn.click(function() {
	stepFromCache(-1).then(function(response) {
	    $sim_step_btn.prop("disabled", false);
	    $play_sim_btn.prop("disabled", false);
	    $input_display_container.animate({scrollLeft: simIndex % 1000 * 30 - $(window).width()/2}, 300);
	    if (simIndex + cache_length - cache_index - 1 == 0)
		$sim_rev_btn.prop("disabled", true);		
	}, function(reject) {
	    console.log(reject);
	});	
    });
    $hidden_play_btn.click(function(){
	stepFromCache(1).then(function(resolve) {
	    $input_display_container.animate({scrollLeft: simIndex % 1000 * 30 - $(window).width()/2}, 0);
	}, function(reject) {
	    //console.log(reject);
	    clearInterval(interval);
	    playSim = false;
	    if (reject == "stop on report") 
		$sim_step_btn.prop("disabled", false);
	    $sim_rev_btn.prop("disabled", false);
	    $play_sim_btn.html("Play Simulation");
	});
    });
    var interval = null;
    $play_sim_btn.click(function() {
	playSim ^= true;
	if (playSim) {
	    $play_sim_btn.html("Stop Simulation <span class='glyphicon glyphicon-pause' aria-hidden='true'></span>");
	    $sim_step_btn.prop("disabled", true);
	    $sim_rev_btn.prop("disabled", true);
	}
	else {
	    $play_sim_btn.html("Play Simulation <span class='glyphicon glyphicon-play' aria-hidden='true'></span>");
	    $sim_step_btn.prop("disabled", false);
	    $sim_rev_btn.prop("disabled", false);
	    clearInterval(interval);
	    return;
	}
	interval = setInterval(function() {
	    $hidden_play_btn.click();
	}, speed*-1000 + 1);

    });

    // Sigma instance
    sig = new sigma({
	renderer: {
	    container: document.getElementById('graph-container')
	    // type: 'webgl'
	},
	graph: {
	    nodes: [
		{
		    id: "_temp_placeholder_node_1",
		    x: 100,
		    y: -100,
		    invisible: true
		},
		{
		    id: "_temp_placeholder_node_2",
		    x: -100,
		    y: 100,
		    invisible: true
		}
	    ],
	    edges: []
	},
	settings: sig_settings
    });

    // Linkurious plugins
    locate_plugin = sigma.plugins.locate(sig);
    tooltips = sigma.plugins.tooltips(sig, sig.renderers[0], tooltip_def_config);

    sig.bind('clickStage', function() {tooltips.close()});

    sig.refresh({skipIndexation: false});

    var colors = ['#eeeeee','#ff0000','#ff8800','#ffff00','#00ff00','#00ffff','#0000ff','#000088'];
    var scale = [0,5,13,23,38,56,77,99];
    heatmap = new HeatMapper(colors, scale);
}

/**
 * Sets the value of 'input_length'
 * @param {Number} length - the value to set 'input_length' to
 */
function setInputLength(length) {
    input_length = length;
}

/**
 * Opens the 'Create STE' modal
 */
function triggerAddSTE() {
    tooltips.close();
    $('#add-ste-modal').modal('show');
}

/**
 * Sends data to C++ to load 'Update STE' modal with
 * @param {String} nodeId - the ID of the node to be edited
 */
function triggerChangeData(nodeId) {
    tooltips.close();
    var node = sig.graph.nodes(nodeId),
    start = (node.data.start) ? node.data.start.substring(12) : "none",
    rep = (node.data.rep_code) ? node.data.rep_code.substring(13) : "";
    editNodeId = nodeId;

    Wt.emit(Wt, "changeSTEData", node.id, node.data.ss, start, rep);
}

/**
 * Triggers deleting an STE from the automata
 * @param {String} nodeId - the ID of the node to be deleted
 */
function triggerDeleteSTE(nodeId) {
    tooltips.close();
    $('#delete-ste-modal').modal('show');
    Wt.emit(Wt, "deleteSTE", nodeId);
}

// **************************** //
// *** SIMULATION FUNCTIONS *** //
// **************************** //

/**
 * Resets all simulation settings
 */
function resetSimulation() {
    $table.find('tr').find('td').removeClass("highlight");
    $table.find('tr').find('td').removeClass("report");
    simIndex = 0;
    cache_length = 0;
    cache_index = -1;
    cache_fetch_size = 100;
    playSim = false;
    updatingCache = false;
    cachedGraphs = {};
    changedGraph = {nodes: [], edges: []};
    reportRecord = "";
    $download_rep_btn.hide();
    textFile = null;
    $table = $('#input-table');
    $('#angle-slider').val(0);
    graph_container.style.height = "calc(100% - " + 
	document.getElementsByClassName("footer")[0].offsetHeight + 
	"px - " + document.getElementById("click-collapse-bar").offsetHeight + 
	"px)";
    sig.graph.nodes().forEach(function(n) {
	n.count = 0;
    });
    toggleHeatMap();
}

/**
 * Loads all new cached graphs, adjusts indexes, sets new clickable range
 * @param {Number} index - the new max index of cached graphs
 * @param {Object} json_object - the JSON object containing all cached graph data
 */
function setCachedGraphs(index, json_object) {
    var prevIndex = cache_index,
    i,
    k;
    // Reset clickability of previous graphs
    $('.input-display-table a').contents().unwrap();
    
    cachedGraphs = json_object;
    cache_length = cachedGraphs[0].length;
    cache_index = index;
    updatingCache = false;
    
    // Make new range of cache clickable
    $table.find('tr').each(function(index, row) {
	$(row).find('td').slice(cache_index - cache_length + 1, cache_index + 1).children().each(function(i, e) {
	    $(e).wrap("<a href='#' onClick='indexClick(" + (cache_index - cache_length + i + 1) + ")'></a>");
	});
    });
    // Update report record and character stream to reflect reports
    for (i = cache_length - cache_index + prevIndex; i<cache_length; i++) {
	// Add reporting nodes to record
	if (cachedGraphs[0][i]["rep_nodes"].length != 0) {
	    $download_rep_btn.show();
	    var abs_i = cache_index - cache_length + i + 1;

	    reportRecord += "Reporting on '" + cachedGraphs[0][i].symbol + "' @cycle " + abs_i + ":\n";
	    var reports = "";
	    var rep_length = cachedGraphs[0][i]["rep_nodes"].length;
	    for (k = 0; k < rep_length; k++) {
		var node = cachedGraphs[0][i]["rep_nodes"][k]; 
		reportRecord += "\tid: " + node.id + "  report code: " + node.rep_code + "\n";
		reports += "<p>" + node.id + ": " + node.rep_code + "</p>";
	    }

	    // Loading UTF and hex row with data

	    $table.find('tr').each(function(i, e) {
		var cell = $(e).find('td').eq(abs_i);
		cell.attr("data-toggle", "popover");
		cell.attr("title", "Reporting STEs @" + abs_i);
		cell.attr("data-content", reports);
		cell.attr("data-trigger", "hover focus");
		cell.attr("data-container", "body");
		cell.attr("data-placement", "top");
		cell.attr("data-html", "true");
		cell.addClass("report");
	    });
	    $('[data-toggle="popover"]').popover();
	}	
    }
    
    $download_rep_btn.click(function() {
	exportFile("reports.txt", reportRecord);
    });
    
    // Enable forward sim buttons
    if (!playSim)
	$sim_step_btn.prop("disabled", false);
    $play_sim_btn.prop("disabled", false);

}

/**
 * Attempts to load a cached graph
 * @param {Number} step_size - the relative size of the step in the cache
 * @returns {Promise} resolves if successful or new cache is being fetched; rejected otherwise
 */
function stepFromCache(step_size) {
    return new Promise(function(resolve, reject) {
	var relativeIndex = simIndex + cache_length - cache_index - 1 + step_size;
	if (relativeIndex >= cache_length || relativeIndex < 0) {
	    if (updatingCache)
		resolve("updating cache...continue simulation");
	    else
		reject("End of cache reached");
	    return;
	}
	// Setting table cells to reflect changed simIndex
	if (simIndex >= 0)
	    $table.find('tr').find('td:eq(' + simIndex + ')').removeClass("highlight");
	 
	simIndex += step_size;
	$('#cycle-index-text').html("" + simIndex);
	
	$table.find('tr').find('td:eq(' + simIndex + ')').addClass("highlight");
	updateGraph(cachedGraphs[0][relativeIndex]);

	// To stop simulation or to continue it
	if (stopSimOnReport && cachedGraphs[0][relativeIndex]["rep_nodes"].length > 0)
	    reject("stop on report");
	else if (stopSimOnCycle == simIndex)
	    reject("stop on cycle");
	else	    
	    resolve("Loaded graph");

	// Cache reloading
	if (!updatingCache && cache_index != input_length - 1 && cache_index - simIndex <= .25 * cache_fetch_size) {
	    updatingCache = true;
	    $('#hidden-cache-btn').click();	   
	}
    });
}

/**
 * Updates graph colors to reflect automata state
 * Logs the time it takes to update the graph to the console
 * @param {Object} updateJson - the data of updated activity types
 */
function updateGraph(updateJson) {

    var timer = new Date().getTime(),
    i,
    nodesLength = updateJson['nodes'].length,
    sigmaNodes,
    outEdges,
    e;

    // Revert the colors of all previously updated nodes and edges
    if (!heatMode) {
	sig.graph.nodes(changedGraph.nodes).forEach(function (n) {
	    n.color = n.originalColor;	  
	});
	sig.graph.edges(changedGraph.edges).forEach(function (e) {
	    delete e.color;
	});
    }
    /*else {
	changeGraph.nodes().forEach(function (n) {
	    var node = sig.graph.nodes(n.id);
	    node.color = "rgb(" + toInt(255 - 255/(node.count+1)) + "," + Math.min(node.count, 255) + ",0)";
	});
    }*/

    // This will only store the IDs of all nodes and edges that were updated
    changedGraph = {nodes: [], edges: []};    
    // Load changedGraph with node IDs from incoming data
    for (i = 0; i < nodesLength; i++)
	changedGraph.nodes.push(updateJson.nodes[i].id);
    // Get array containing references to actual graph nodes
    sigmaNodes = sig.graph.nodes(changedGraph.nodes);

    // Update these nodes
    for (i = 0; i < nodesLength; i++) {
	// Update count only if it is higher than before -- no backwards traversal for heat map
	sigmaNodes[i].count = Math.max(parseInt(updateJson.nodes[i].count), sigmaNodes[i].count);
	if (heatMode) 
	    sigmaNodes[i].color = heatmap.getHexColor(sigmaNodes[i].count, true);
	else
	    sigmaNodes[i].color = getColor(updateJson.nodes[i].activity);
	// If node is activated, light up outgoing edges
	if (updateJson.nodes[i].activity == "activated" && !heatMode) {
	    outEdges = sig.graph.outEdges(sigmaNodes[i].id);
	    for (e in outEdges) {
		outEdges[e].color = page_settings['activatedSTEColor'];
		changedGraph.edges.push(e);
	    }
	}
    }
    sig.refresh({skipIndexation: true});
    console.log("update graph timer: " + (new Date().getTime() - timer));
}


// ************************************ //
// *** GRAPH MANIPULATION FUNCTIONS *** //
// ************************************ //

/**
 * Changes the size of all edges on screen with geometric equation
 * @param {Number} val - the value from the edge slider
 */
function edgeSizeChange(val) {
    edgeSize = .005 * Math.pow(1.2, val);   
    // if (editor_mode)
	sig.graph.edges().forEach(function (e) {
	    e.size = edgeSize * elemScalar;
	});
    sig.settings('maxEdgeSize', edgeSize);
    sig.refresh({skipIndexation: true});
}

/**
 * Changes the size of all nodes on screen with geometric equation
 * @param {Number} val - the value from the node slider
 */
function nodeSizeChange(val) {
    nodeSize = .1 * Math.pow(1.1, val);
    //if (editor_mode)
	sig.graph.nodes().forEach(function (n) {
	    n.size = nodeSize * elemScalar;
	});
    sig.settings('maxNodeSize', nodeSize);
    sig.settings('zoomMin', nodeSize / 80 * elemScalar);
    sig.refresh({skipIndexation: true});
}

/**
 * Resets the camera and autoscales if necessary
 */
function resetCamera() {
    tooltips.close();
    var camPos = {ratio: 1, x: 0, y: 0, angle: 0},
    cam = sig.cameras[0];
    if (!sig.settings("autoRescale")) {
	bounds = sigma.utils.getBoundaries(sig.graph, "", true, true);
	var minX = (bounds.minX == "Infinity") ? -1 : bounds.minX,
	minY = (bounds.minY == "Infinity") ? -1 : bounds.minY,
	maxX = (bounds.maxX == "-Infinity") ? 1 : bounds.maxX,
	maxY = (bounds.maxY == "-Infinity") ? 1 : bounds.maxY,
	w = sig.renderersPerCamera[cam.id][0].width || 1,
	h = sig.renderersPerCamera[cam.id][0].height || 1,
	scale = Math.min(w / Math.max(maxX - minX, 1), h / Math.max(maxY - minY, 1)),
	margin = nodeSize / scale + +sig.settings('sideMargin');
	maxX += margin;
	maxY += margin;
	minX -= margin;
	minY -= margin;
	scale = Math.min(w / Math.max(maxX - minX, 1), h / Math.max(maxY - minY, 1));
	camPos = {x: (maxX + minX) / 2, 
		  y: (maxY + minY) / 2, 
		  ratio: 1 / scale, 
		  angle: 0};
    }
    sigma.misc.animation.camera(
	cam,
	camPos,
	{ duration: 150 }
    );
    $('#angle-slider').val(0);
}

/**
 * Rotates the camera to the number of degrees input
 * @param {Number} val - the angle to which the camera is rotated
 */
function rotationChange(val) {
    sig.cameras[0].goTo({angle: -1 * Math.PI / 180 * +val});
    sig.refresh({skipIndexation: true});
}

/**
 * Hides or shows edges based on 'Render Edge' check state
 */
function toggleEdges() {
    sig.settings("drawEdges", $('#inrender-edge-box').is(':checked'));
    $('#edge-slider').toggle('disabled');
    draw_edges ^= true;
    sig.refresh({skipIndexation: true});
}

/**
 * Toggles heat map mode
 */
function toggleHeatMap() {
    if ($('#inheat-mode-box').is(':checked')) {
	heatMode = true;
	sig.graph.nodes().forEach(function (n) {
	    n.color = heatmap.getHexColor(n.count, true);
	});
	sig.graph.edges().forEach(function (e) {
	    delete e.color;
	});
    }
    else {
	heatMode = false;
	sig.graph.nodes().forEach(function (n) {
	    n.color = n.originalColor;
	});
    }

    sig.refresh({skipIndexation: true});
}

/**
 * Changes the width ratio of the graph geometrically by scaling
 * the x coordinate of all nodes
 * @param {Number} val - the value from the width ratio slider
 */
function widthChange(val) {
    var ratio = Math.pow(1.1, val - graphWidth);
    sig.graph.nodes().forEach(function (n) {
	n.x *= ratio;
    });
    graphWidth = val;
    sig.refresh({skipIndexation: true});
}


// ***************************** //
// *** EDITOR MODE FUNCTIONS *** //
// ***************************** //


/**
 * Adds a new edge to the graph where the user clicks next
 * If the user doesn't click a node, the edge is deleted
 * @param {String} sourceId - the ID of the source node of the edge
 */
function addEdge(sourceId) {
    tooltips.close();
    
    dragListener.addEdge(sourceId, edgeSize * elemScalar);
    select.addEdge(sourceId, edgeSize * elemScalar);

    automataChanged = true;
}

/**
 * Adds a new node to the graph where the user clicks next
 */
function addSTE(id, ss, rep_code, start, type) {
    $('#add-ste-modal').modal('hide');

    var newNode = {
	id: id,
	x: 1,
	y: 1,
	size: nodeSize * elemScalar,
	color: getColor(type),
	originalColor: getColor(type),
	data: {
	    ss: ss,
	    rep_code: rep_code,
	    start: start,
	    type: type
	}
    };
    
    // First STE added in default location
    if (sig.graph.visibleNodes().length == 0) {
	// Max out size options when starting out
	var $nodeSlider = $('#node-slider');
	$nodeSlider.val($nodeSlider.attr("max"));
	nodeSizeChange($nodeSlider.val());
	var $edgeSlider = $('#edge-slider');
	$edgeSlider.val($edgeSlider.attr("max"));
	edgeSizeChange($edgeSlider.val());

	newNode.size = nodeSize * elemScalar;

	// First node is centered, others can be used as reference points in dragNodes
	// They cannot be on the same axis
	newNode.x = 0;
	newNode.y = 0;
    }

    // Allow user to place this node
    dragListener.addNode(newNode);

    automataChanged = true;
    
    sig.refresh({skipIndexation: false});    
}

/**
 * Deletes a node from the graph
 * @param {String} nodeId - the ID of the node to be deleted
 */
function deleteSTE(nodeId) {
    $('#delete-ste-modal').modal('hide');    
    sig.graph.dropNode(nodeId);
    automataChanged = true;
    sig.refresh({skipIndexation: false});
}

/**
 * Removes an edge from the graph and the automata
 * @param {String} edgeId - the ID of the edge to be deleted
 */
function removeEdge(edgeId) {
    tooltips.close();

    var edge = sig.graph.edges(edgeId);
    sig.graph.dropEdge(edgeId);
    Wt.emit(Wt, "toggleConnection", edge.source, edge.target, false);
    automataChanged = true;
    sig.refresh({skipIndexation: false});
}

/**
 * Converts the graph data to a string and saves it to a file
 */
function saveGraphData() {
    var nodes = [],
    edges = [],
    graph = {};
    sig.graph.visibleNodes().forEach(function (n) {
	nodes.push({
	    id: n.id,
	    data: n.data,
	    x: n.x / Math.pow(1.1, graphWidth),
	    y: n.y
	});
    });
    sig.graph.edges().forEach(function (e) {
	edges.push({
	    id: e.id,
	    source: e.source,
	    target: e.target
	});
    });

    graph['nodes'] = nodes;
    graph['edges'] = edges;

    exportFile('graph_data.json', JSON.stringify(graph));
}

/**
 * Selects the nodes at either ends of the edge
 * @param {String} edgeId - the ID of the edge
 */
function selectConnectedNodes(edgeId) {
    tooltips.close();

    var edge = sig.graph.edges(edgeId);
    activeState.addNodes([edge.source, edge.target]);
    sig.refresh({skipIndexation: true});
}

/** 
 * Enters into or exits Editor Mode
 * Scales graph appropriately for seamless transitioning
 * Disables simulation tools and steps
 * Instantiates or disables editor plugins
 */
function toggleEditorMode() {
    tooltips.close();
    // For correctly scaling/positioning the camera when switching between modes
    // This code is tweaked from sigma.middlewares.rescale and 
    // emulates the autoscale functionality when switching to editor mode
    bounds = sigma.utils.getBoundaries(sig.graph, "", true, true);
    var cam = sig.cameras[0],
    // The changes to min and max allow for editor mode to be started with an empty graph
    minX = (bounds.minX == "Infinity") ? -1 : bounds.minX,
    minY = (bounds.minY == "Infinity") ? -1 : bounds.minY,
    maxX = (bounds.maxX == "-Infinity") ? 1 : bounds.maxX,
    maxY = (bounds.maxY == "-Infinity") ? 1 : bounds.maxY,
    w = sig.renderersPerCamera[cam.id][0].width || 1,
    h = sig.renderersPerCamera[cam.id][0].height || 1,
    scale = Math.min(w / Math.max(maxX - minX, 1), h / Math.max(maxY - minY, 1)),
    margin = nodeSize / scale + +sig.settings('sideMargin');
    maxX += margin;
    maxY += margin;
    minX -= margin;
    minY -= margin;
    scale = Math.min(w / Math.max(maxX - minX, 1), h / Math.max(maxY - minY, 1));

    /* Start Editor mode */
    if (!editor_mode) {
	editor_mode = true;

	// Hide and disable header
	$('#collapseHeader').collapse('hide');
	$('#nav-hide-icon').unwrap();
	toggleChevron();
	
	// Disable clickability of character stream
	$('.input-display-table a').contents().unwrap();

	// Set background of the graph container
	graph_container.style.backgroundColor = page_settings['editModeBGColor'];

	// Show Editor Tab and hide Simulation Tools
	$('#editor-tab').show();
	$('#editor-tab').addClass('active');
	$('#editor-tools').show();
	
	$('#graph-settings').hide();
	$('#graph-tab').removeClass('active');
	$('#sim-tab').hide();
	$('#sim-tools').hide();
	$('#sim-tab').removeClass('active');

	// Appropriately display nodes and edges without autoscaling
	// Undo transformations by reversing code from sigma.middlewares.rescale
	var camSettings = {x: +cam.x / scale + (maxX + minX) / 2, 
			   y: +cam.y / scale + (maxY + minY) / 2, 
			   ratio: cam.ratio / scale, 
			   angle: cam.angle};
	
	elemScalar = 1.0 / scale;

	// Set node color and size
	sig.graph.nodes().forEach(function(n) {
	    n.color = n.originalColor;
	    n.size = nodeSize * elemScalar;
	    n.hidden = false;
	});
	
	sig.graph.edges().forEach(function(e) {
	    e.size = edgeSize * elemScalar;
	    delete e.color;
	});
	
	// Switch to Canvas renderer
	sig.killRenderer(sig.renderers[0]);
	sig.killCamera(sig.cameras[0]);
	sig.addRenderer({
	    container: document.getElementById('graph-container'),
	    type: 'canvas'
	});
	sig.cameras[0].goTo(camSettings);

	// Turn off autoRescale because it's annoying when moving nodes
	sig.settings('autoRescale', false);
	// Enable edge hovering to allow user to delete connections
	sig.settings('enableEdgeHovering', true);

	// Instantiate plug-ins
	var renderer = sig.renderers[0];
	activeState = sigma.plugins.activeState(sig);
	dragListener = sigma.plugins.dragNodes(sig, renderer, activeState);
	select = sigma.plugins.select(sig, activeState, renderer);
	keyboard = sigma.plugins.keyboard(sig, renderer);
	select.bindKeyboard(keyboard);
	rectSelect = new sigma.plugins.rectSelect(sig, renderer);
	lasso = new sigma.plugins.lasso(sig, renderer, {
	    strokeStyle: page_settings['lassoToolStrokeColor'],
	    fillWhileDrawing: true
	});
	select.bindLasso(lasso);
	select.bindRect(rectSelect);

	// Change tooltip menu to edit features
	sigma.plugins.killTooltips(sig);
	tooltips = sigma.plugins.tooltips(sig, renderer, tooltip_edit_config);
	
	// Refresh the view
	sig.refresh({skipIndexation: false});

    }
    /* End Editor Mode */
    else {
	editor_mode = false;

	// Revert background color
	graph_container.style.backgroundColor = page_settings['stageBGColor'];

	// Toggle tabs
	$('#editor-tab').hide();
	$('#editor-tab').removeClass('active');
	$('#editor-tools').hide();
	
	$('#graph-settings').show();
	$('#graph-tab').addClass('active');
	if (cache_length > 0) 
	    $('#sim-tab').show();

	// Kill editor plugins
	sigma.plugins.killDragNodes(sig);
	sigma.plugins.killActiveState();
	sigma.plugins.killSelect(sig);
	sigma.plugins.killLasso(sig);
	sigma.plugins.killRect(sig);
	
	// Revert Tooltip menu
	sigma.plugins.killTooltips(sig);
	tooltips = sigma.plugins.tooltips(sig, sig.renderers[0], tooltip_def_config);

	// Remove renderer data from nodes
	var prefix = sig.renderers[0].options.prefix;
	sig.graph.nodes().forEach(function (n) {
	    delete n[prefix + 'size'];
	    delete n[prefix + 'x'];
	    delete n[prefix + 'y'];
	});

	// Revert camera settings 
	var camSettings = {x: (+cam.x - (maxX + minX) / 2) * scale, 
			   y: (+cam.y - (maxY + minY) / 2) * scale, 
			   ratio: cam.ratio * scale, 
			   angle: cam.angle};

	elemScalar = 1;

	sig.graph.nodes().forEach(function(n) {
	    n.size = nodeSize * elemScalar;
	});	
	sig.graph.edges().forEach(function(e) {
	    e.size = edgeSize * elemScalar;
	});

	// Switch to WebGL renderer
	sig.killRenderer(sig.renderers[0]);
	sig.killCamera(sig.cameras[0]);
	sig.addRenderer({
	    container: document.getElementById('graph-container')
	});

	sig.cameras[0].goTo(camSettings);

	// Turn autoRescale back on
	sig.settings('autoRescale', ['nodePosition']);
	// Turn edge hovering off
	sig.settings('enableEdgeHovering', false);

	if (!automataChanged) { // If the core structure of the automata wasn't changed, do not reset simulation
	    // Reenable collapsable header
	    $('#nav-hide-icon').wrap('<a aria-controls="collapseHeader" ' +
				     'aria-expanded="true" data-toggle="collapse" ' +
				     'href="#collapseHeader" onClick="toggleChevron()" ' + 
				     'class="hide-header-btn"></a>');

	    // Reenable clickable character stream
	    $table.find('tr').each(function(index, row) {
		$(row).find('td').slice(cache_index - cache_length + 1, cache_index + 1).children().each(function(i, e) {
		    $(e).wrap("<a href='#' onClick='indexClick(" + (cache_index - cache_length + i + 1) + ")'></a>");
		});
	    });

	    // Resume simulation
	    if (cache_index >= 0)
		stepFromCache(0);
	} 
	else { // Automata was changed; reset simulation
	    $('#reset-sim-btn').click();
	}

	automataChanged = false;
	sig.refresh({skipIndexation: false});
    }    
}

/**
 * Activates or deactivates the lasso tool
 * Deactivates 'Box Select' tool
 */
function toggleLasso() {
    rectSelect.stop();

    if (lasso.isActive) 
	lasso.deactivate();
    else
	lasso.activate();
}

/**
 * Activates or deactivates the 'Box Select' tool
 * Deactivates lasso tool
 */
function toggleRect() {
    lasso.deactivate();

    if (rectSelect.isActive) 
	rectSelect.stop();
    else
	rectSelect.start();
}

/**
 * Sends the new data to the graph helper function to update the node
 * @param {String} id - the updated ID of the STE
 * @param {String} ss - the updated symbol set of the STE
 * @param {String} start - the updated start type data of the STE
 * @param {String} rep - the updated report data of the STE
 * @param {String} type - the update type of the STE
 */
function updateSTEData(id, ss, start, rep, type) {
    $('#add-ste-modal').modal('hide');
    var data = {
	id: id,
	ss: ss,
	start: start,
	rep_code: rep,
	type: type
    };
    sig.graph.changeData(editNodeId, data);
}

// *********************** //
// *** STAGE FUNCTIONS *** //
// *********************** //

/**
 * Opens the 'Search by ID' modal
 */
function openSearchBar() {
    tooltips.close();
    $('#search-modal').modal('show');
}

/**
 * Logs all node data to the console
 */
function printAllNodeData() {
    tooltips.close();
    sig.graph.nodes().forEach(function(n) {
	/* DON'T REMOVE THIS */ console.log(n);
    });
}

/**
 * Logs the node data of the specified node to the console
 * @param {String} nodeId - the ID of the node
 */
function printNodeData(nodeId) {
    tooltips.close();
    /* DON'T REMOVE THIS */ console.log(sig.graph.nodes(nodeId));
}

/**
 * Zooms to the node whose ID is entered into the search bar
 */
function searchById() {
    var bar = document.getElementById("search-bar");
    locate_plugin.nodes(bar.value);
    bar.value = "";
    
    sig.refresh({skipIndexation: true});    
}

/**
 * Shows all nodes in the graph -- sets hidden to false
 */
function showAllNodes() {
    tooltips.close();
    sig.graph.visibleNodes().forEach(function(n) {
	n.hidden = false;
    });

    sig.refresh({skipIndexation: true});
}

/**
 * Displays only the children of the selected node on the graph
 * @param {String} nodeId - the ID of the parent node
 */
function soloChildren(nodeId) {
    tooltips.close();
    var toKeep = sig.graph.children(nodeId, {});
    toKeep[nodeId] = sig.graph.nodes(nodeId);
 
    sig.graph.nodes().forEach(function(n) {
	if (toKeep[n.id])
	    n.hidden = false;
	else
	    n.hidden = true;
    });

    sig.refresh({skipIndexation: true});
}

/**
 * Hides or shows all the children of the selected node
 * @param {String} nodeId - the ID of the parent node
 */
function toggleChildren(nodeId) {
    tooltips.close();
    var toKeep = sig.graph.children(nodeId, {});

    var hidden = true,
    k;
    // If any are visible, hide all; else show all
    for (k in toKeep)
	if (!toKeep[k].hidden && k != nodeId) {
	    hidden = false;
	    break;
	}
 
    sig.graph.nodes().forEach(function(n) {
	if (toKeep[n.id] && nodeId !=  n.id)
	    n.hidden = !hidden;
    });

    sig.refresh({skipIndexation: true});
}
