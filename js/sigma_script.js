
// Graph neighbor function from Sigma.js
sigma.classes.graph.addMethod('neighbors', function(nodeId, connectedNodes) {

    var k,
    index = this.outNeighborsIndex[nodeId] || {};
    connectedNodes[nodeId] = sig.graph.nodes(nodeId);

    for (k in index) {
	if (!connectedNodes[k])
	    connectedNodes = sig.graph.neighbors(k, connectedNodes);
    }
    return connectedNodes;    

});

function recursiveOutNeighbor(nodeId, connectedNodes) {
    var k,
    index = this.outNeighborsIndex[nodeId] || {};
    connectedNodes[nodeId] = sig.graph.nodes(nodeId);

    for (k in index) {
	if (!connectedNodes[k])
	    connectedNodes = recursiveOutNeighbor(k, connectedNodes);
    }
    return connectedNodes;    
}


// Modified Graph neighbor function from Sigma.js
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

function toInt(n){ return Math.round(Number(n)); };

var changedGraph = new sigma.classes.graph();
var sig = new sigma();
var cachedGraphs;
var initialLoad = true;

// Graph settings variables
var heatMode = false;
var draw_edges = true;
var nodeSize = 1;
var arrowSize = 5;
var edgeSize = .1;
var graphWidth = 1;
var graphHeight = 10;

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

// DOM elements
var $download_rep_btn; 
var $play_sim_btn;
var $sim_step_btn;
var $sim_rev_btn;
var $hidden_play_btn;
var $input_display_container;
var $table;

// Called on page load
function pageLoad() {

    $download_rep_btn = $('#dl-rep-btn');
    $play_sim_btn = $('#play-sim-btn');
    $sim_step_btn = $('#sim-step-btn');
    $sim_rev_btn = $('#sim-rev-btn');
    $hidden_play_btn = $('#hidden-play-btn');
    $input_display_container = $('#input-display-container');

    $table = $("#input-table");

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
    $('#inhex-mode-box').click(function() {
	$table.find('tr').toggle();
    });
    $('#instop-sim-report-box').click(function() {
	stopSimOnReport = document.getElementById('instop-sim-report-box').checked;
    });
    $('#reset-camera-btn').click(function() {
	sigma.misc.animation.camera(
	    sig.cameras[0],
	    { ratio: 1, x: 0, y: 0, angle: 0 },
	    { duration: 150 }
	);
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
	    $play_sim_btn.html("Stop Simulation");
	    $sim_step_btn.prop("disabled", true);
	    $sim_rev_btn.prop("disabled", true);
	}
	else {
	    $play_sim_btn.html("Play Simulation");
	    $sim_step_btn.prop("disabled", false);
	    $sim_rev_btn.prop("disabled", false);
	    clearInterval(interval);
	    return;
	}
	interval = setInterval(function() {
	    $hidden_play_btn.click();
	}, speed*-1000 + 1);

    });

    sig = new sigma({
	renderer: {

	    container: document.getElementById('graph-container'),
	    // Allows for alpha channel and self-loops; uses WebGL if possible, canvas otherwise
	    // type: 'canvas'
	},
	settings: {
	    skipIndexation: true,
	    labelThreshold: 100,
	    hideEdgesOnMove: true,
	    //batchEdgesDrawing: true, doesn't render edges farther in graph
	    zoomMin: .00001,
	    zoomMax: 2,
	    edgeColor: "default",
	    drawEdges: draw_edges,
	    defaultEdgeColor: "#888",
	    defaultEdgeType: "arrow",
	    defaultNodeType: "fast",
	    maxNodeSize: nodeSize,
	    minNodeSize: 0,
	    minEdgeSize: 0,
	    maxEdgeSize: edgeSize,
	    minArrowSize: arrowSize,
	    nodesPowRatio: 1,
	    edgesPowRatio: 1
	}
    });

    // Neighborhood clickability

    sig.bind('clickNode', function(e) {
	var nodeId = e.data.node.id,
	toKeep = sig.graph.neighbors(nodeId, {});
	toKeep[nodeId] = e.data.node;

	sig.graph.nodes().forEach(function(n) {
	    if (toKeep[n.id])
		n.hidden = false;
	    else
		n.hidden = true;
	});

	sig.refresh({skipIndexation: true});
    });

    sig.bind('clickStage', function(e) {
	sig.graph.nodes().forEach(function(n) {
	    n.hidden = false;
	});

	sig.refresh({skipIndexation: true});
    });
    
}

function resetSimulation() {
    simIndex = 0;
    cache_length = 0;
    cache_index = -1;
    cache_fetch_size = 100;
    playSim = false;
    updatingCache = false;
    cachedGraphs = {};
    changedGraph.clear();
    reportRecord = "";
    $download_rep_btn.hide();
    textFile = null;
    $table = $('#input-table');
}

function setInputLength(length) {
    input_length = length;
}

function makeTextFile() {
    var data = new Blob([reportRecord], {type: 'text/plain'});

    // If we are replacing a previously generated file we need to
    // manually revoke the object URL to avoid memory leaks.
    if (textFile !== null) {
	window.URL.revokeObjectURL(textFile);
    }

    textFile = window.URL.createObjectURL(data);

    return textFile;
}

function setCachedGraphs(index, json_object) {
    var prevIndex = cache_index,
    i,
    k;
    // Reset clickability of previous graphs
    $('a.click-index').contents().unwrap();
    
    cachedGraphs = json_object;
    cache_length = cachedGraphs[0].length;
    cache_index = index;
    updatingCache = false;
    
    // Make new range of cache clickable
    $table.find('tr').each(function(index, row) {
	$(row).find('td').slice(cache_index - cache_length + 1, cache_index + 1).children().each(function(i, e) {
	    $(e).wrap("<a href='#' class='click-index' onClick='indexClick(" + i + ")'></a>");
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
    
    $download_rep_btn.attr({href: makeTextFile()});
    
    // Enable forward sim buttons
    if (!playSim)
	$sim_step_btn.prop("disabled", false);
    $play_sim_btn.prop("disabled", false);

}

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

function loadGraph(json_object){
    
    nodeSizeChange($('#node-slider').val());    
    edgeSizeChange($('#edge-slider').val());
    widthChange($('#width-slider').val());
     
    $download_rep_btn.hide();

    try {
	sig.graph.clear();
	sig.refresh();
    } catch (e) {
	console.log("Error clearing graph: " + e.message);
    }
    try {
	
	sig.graph.read(json_object);
	
	// Set sizes and save color of nodes

	sig.graph.nodes().forEach(function(n) {
	    n.size = "1";
	    n.originalColor = n.color;
	    n.ox = n.x;
	    n.x = n.x * graphWidth;
	    n.count = 0;
	    if (heatMode)
		n.color = "rgb(0,0,0)";
	});
	sig.graph.edges().forEach(function(e) {
	    e.size = "0.05";
	});

	sig.refresh();
	$('#loading-graph-modal').modal('hide');
    } catch (err) {
	$('#loading-graph-modal').modal('hide');
	alert(err.message);
    }
    
}

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

	if (stopSimOnReport && cachedGraphs[0][relativeIndex]["rep_nodes"].length > 0)
	    reject("stop on report");
	else	    
	    resolve("Loaded graph");

	// Cache reloading
	if (!updatingCache && cache_index != input_length - 1 && cache_index - simIndex <= .25 * cache_fetch_size) {
	    updatingCache = true;
	    $('#hidden-cache-btn').click();	   
	}
    });
}

function updateGraph(updateJson) {
    // Revert the colors of all previously updated nodes and 
    //console.log(JSON.stringify(updateJson));
    var timer = new Date().getTime();
    if (!heatMode) {
	changedGraph.nodes().forEach(function (n) {
	    sig.graph.nodes(n.id).color = n.originalColor;
	});
	changedGraph.edges().forEach(function (e) {
	    sig.graph.edges(e.id).color = "";
	});
    }
    /*
      else {
      changeGraph.nodes().forEach(function (n) {
      var node = sig.graph.nodes(n.id);
      node.color = "rgb(" + toInt(255 - 255/(node.count+1)) + "," + Math.min(node.count, 255) + ",0)";
      });
      }*/
    changedGraph.clear();
    
    var changedNodesArray = updateJson.nodes;
    // Make an array of node IDs from incoming data
    var changedNodeIDs = [];
    for (var i = 0; i < changedNodesArray.length; i++)
	changedNodeIDs[i] = changedNodesArray[i].id;
    // Get array containing references to actual graph nodes
    var sigmaNodes = sig.graph.nodes(changedNodeIDs);
    // Add these nodes to changedGraph nodes
    for (var i = 0; i < sigmaNodes.length; i++)
	changedGraph.addNode(sigmaNodes[i]);
    for (var i = 0; i < sigmaNodes.length; i++) {
	// Update count only if it is higher than before -- no backwards traversal for heat map
	sigmaNodes[i].count = Math.max(parseInt(changedNodesArray[i].count), sigmaNodes[i].count);
	if (heatMode) 
	    sigmaNodes[i].color = "rgb(" + toInt(255 - 255/(sigmaNodes[i].count+1)) + "," + Math.min(sigmaNodes[i].count, 255) + ",0)";
	else
	    sigmaNodes[i].color = changedNodesArray[i].color;
	// If node is activated, light up outgoing edges
	if (sigmaNodes[i].color == "rgb(0,255,0)" && !heatMode) {
	    var outgoingEdges = sig.graph.outEdges(sigmaNodes[i].id);
	    for (e in outgoingEdges) {
		outgoingEdges[e].color = "#0f0";
		try {
		    changedGraph.addEdge(outgoingEdges[e]);
		} catch (error) {
		    console.log(error);
		    console.log(outgoingEdges[e]);
		}
	    }
	}
    }
    sig.refresh({skipIndexation: true});
    console.log("update graph timer: " + (new Date().getTime() - timer));
}

// Graph manipulation controls

function nodeSizeChange(val) {
    nodeSize = .1 * Math.pow(1.1, val);
    sig.settings('maxNodeSize', nodeSize);
    sig.refresh({skipIndexation: true});
}

function edgeSizeChange(val) {
    edgeSize = .005 * Math.pow(1.2, val);
    sig.settings('maxEdgeSize', edgeSize);
    sig.refresh({skipIndexation: true});
}

function toggleEdges() {
    if ($('#inrender-edge-box').is(':checked'))
	sig.settings("drawEdges", true);
    else 
	sig.settings("drawEdges", false);

    $('#edge-slider').toggle('disabled');
    draw_edges ^= true;
    sig.refresh({skipIndexation: true});
}

function toggleHeatMap() {
    if ($('#inheat-mode-box').is(':checked')) {
	heatMode = true;
	sig.graph.nodes().forEach(function (n) {
	    n.color = "rgb(" + toInt(255 - 255/(n.count+1)) + "," + Math.min(n.count, 255) + ",0)";
	    //console.log(n.color);
	});
	sig.graph.edges().forEach(function (e) {
	    e.color = "#888";
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

function widthChange(val) {
    graphWidth = Math.pow(1.1, val);
    sig.graph.nodes().forEach(function (n) {
	n.x = n.ox * graphWidth;
    });
    sig.refresh();
}
