
// Graph neighbor function from Sigma.js
sigma.classes.graph.addMethod('neighbors', function(nodeId) {
    var k,
    neighbors = {},
    index = this.allNeighborsIndex[nodeId] || {};

    for (k in index) {
        neighbors[k] = this.nodesIndex[k];
    }
    return neighbors;
});


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

var heatMode = false;
var draw_edges = true;
var nodeSize = 1;
var arrowSize = 5;
var edgeSize = .1;
var graphWidth = 1;

var simIndex = 0;
var cache_length = 0;
var cache_index = -1;
var cache_fetch_size = 100;
var playSim = false;
var speed = 0;
var updatingCache = false;
var input_length = 0;

function setInputLength(length) {
    input_length = length;
}

function setCachedGraphs(index, json_object) {
    cachedGraphs = json_object;
    cache_length = cachedGraphs[0].length;
    cache_index = index;
    updatingCache = false;
}

function loadCachedGraph(index) {
    var graph = cachedGraphs[0][index];
    updateGraph(graph);
}

function loadGraph(json_object){
    // Reset all simulation settings when new graph loaded
    simIndex = 0;
    cache_length = 0;
    cache_index = -1;
    cache_fetch_size = 100;
    playSim = false;
    updatingCache = false;

    nodeSizeChange($('#node-slider').val());
    arrowSizeChange($('#arrow-slider').val());
    edgeSizeChange($('#edge-slider').val());
    widthChange($('#width-slider').val());

    try {
	sig.graph.clear();
	sig.refresh();
    } catch (e) {
	console.log("Error clearing graph: " + e.message);
    }
    try {
	sig = new sigma({
	    renderer: {

		container: document.getElementById('graph-container'),
		// Allows for alpha channel and self-loops; uses WebGL if possible, canvas otherwise
		//type: 'canvas'
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
		maxNodeSize: nodeSize,
		minNodeSize: 0,
		minEdgeSize: 0,
		maxEdgeSize: edgeSize,
		minArrowSize: arrowSize
	    }
	});
	sig.graph.read(json_object);
	
	// Set sizes and save color of nodes

	sig.graph.nodes().forEach(function(n) {
	    n.size = "1";
	    n.originalColor = n.color;
	    n.oy = n.y;
	    n.y = n.y * graphWidth;
	    n.count = 0;
	    if (heatMode)
		n.color = "rgb(0,0,0)";
	});
	sig.graph.edges().forEach(function(e) {
	    e.size = "0.05";
	});

	// Neighborhood clickability

	sig.bind('clickNode', function(e) {

	    var nodeId = e.data.node.id,
	    toKeep = sig.graph.neighbors(nodeId);
	    toKeep[nodeId] = e.data.node;

	    sig.graph.nodes().forEach(function(n) {
		if (toKeep[n.id])
		    n.hidden = false;
		else
		    n.hidden = true;
	    });

	    sig.graph.edges().forEach(function(e) {
		if (e.source == nodeId || e.target == nodeId) 
		    e.hidden = false;
		else
		    e.hidden = true;
	    });

	    sig.refresh();
	});

	sig.bind('clickStage', function(e) {
	    sig.graph.nodes().forEach(function(n) {
		n.hidden = false;
	    });

	    sig.graph.edges().forEach(function(e) {
		e.hidden = false;
	    });

	    sig.refresh();
	});
	
	CustomShapes.init(sig);
	sig.refresh();
	$('#loading-graph-modal').modal('hide');
    } catch (err) {
	$('#loading-graph-modal').modal('hide');
	alert(err.message);
    }

    //  Button listeners active after graph load
    $('input[type=range]').on('input', function() {
	$(this).trigger('change');
    });
    $('#node-slider').change( function() {
	nodeSizeChange($(this).val());
    });
    $('#arrow-slider').change( function() {
	arrowSizeChange($(this).val());
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
	    $('#play-sim-btn').html("Play Simulation");
	    $('#sim-step-btn').prop("disabled", false);
	    $('#sim-rev-btn').prop("disabled", false);
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
    $('#reset-camera-btn').click(function() {
	sigma.misc.animation.camera(
	    sig.cameras[0],
	    { ratio: 1, x: 0, y: 0, angle: 0 },
	    { duration: 150 }
	);
    });
    $('#sim-step-btn').click(function() {
	stepFromCache(1).then(function(response) {
	    $('#sim-rev-btn').prop("disabled", false);
	    $('#input-display-container').animate({scrollLeft: simIndex % 1000 * 30 - $(window).width()/2}, 300);
	    if (simIndex == cache_index)
		$('#sim-step-btn').prop("disabled", true);
	}, function(reject) {
	    console.log(reject);
	});
    });
    $('#sim-rev-btn').prop("disabled", true);
    $('#sim-rev-btn').click(function() {
	stepFromCache(-1).then(function(response) {
	    $('#sim-step-btn').prop("disabled", false);
	    $('#input-display-container').animate({scrollLeft: simIndex % 1000 * 30 - $(window).width()/2}, 300);
	    if (simIndex + cache_length - cache_index - 1 == 0)
		$('#sim-rev-btn').prop("disabled", true);		
	}, function(reject) {
	    console.log(reject);
	});
	
    });
    $('#hidden-play-btn').click(function(){
	stepFromCache(1).then(function(response) {
	    $('#input-display-container').animate({scrollLeft: simIndex % 1000 * 30 - $(window).width()/2}, 0);
	}, function(reject) {
	    console.log(reject);
	    clearInterval(interval);
	    playSim = false;
	    $('#sim-rev-btn').prop("disabled", false);
	    $('#play-sim-btn').html("Play Simulation");
	});

    });
    var interval = null;
    $('#play-sim-btn').click(function() {
	playSim ^= true;
	if (playSim) {
	    $('#play-sim-btn').html("Stop Simulation");
	    $('#sim-step-btn').prop("disabled", true);
	    $('#sim-rev-btn').prop("disabled", true);
	}
	else {
	    $('#play-sim-btn').html("Play Simulation");
	    $('#sim-step-btn').prop("disabled", false);
	    $('#sim-rev-btn').prop("disabled", false);
	    clearInterval(interval);
	    return;
	}
	interval = setInterval(function() {
	    $('#hidden-play-btn').click();
	}, speed*-1000 + 1);

    });
}

function stepFromCache(step_size) {
    return new Promise(function(resolve, reject) {
	var relativeIndex = simIndex + cache_length - cache_index - 1 + step_size;
	if (relativeIndex >= cache_length || relativeIndex < 0) {
	    reject("End of cache reached");
	    return;
	}
	var table = document.getElementById("input-table");
	if (simIndex >= 0) {
	    table.rows[0].cells[simIndex].classList.remove("highlight");
	    table.rows[1].cells[simIndex].classList.remove("highlight");	
	}
	simIndex += step_size;

	table.rows[0].cells[simIndex].classList.add("highlight");
	table.rows[1].cells[simIndex].classList.add("highlight");
	

	if (step_size > 0)
	    for (var i = simIndex - step_size + 1; i <= simIndex; i++) {
		var i_relativeIndex = i + cache_length - cache_index - 1;
		if (cachedGraphs[0][i_relativeIndex]["rep_nodes"].length != 0) {
		    var reports = "";
		    for (var k = 0; k < cachedGraphs[0][i_relativeIndex]["rep_nodes"].length; k++) {
			var node = cachedGraphs[0][i_relativeIndex]["rep_nodes"][k];
			//console.log(node);
			reports += "<p>" + node.id + ": " + node.rep_code + "</p>";
		    }
		    // Loading UTF and hex row with data
		    for (var k = 0; k <= 1; k++) {
			table.rows[k].cells[i].setAttribute("data-toggle", "popover");
			table.rows[k].cells[i].setAttribute("title", "Reporting STEs");
			table.rows[k].cells[i].setAttribute("data-content", reports);
			table.rows[k].cells[i].setAttribute("data-trigger", "hover focus");
			table.rows[k].cells[i].setAttribute("data-container", "body");
			table.rows[k].cells[i].setAttribute("data-placement", "top");
			table.rows[k].cells[i].setAttribute("data-html", "true");
			table.rows[k].cells[i].classList.add("report");
		    }
		    $('[data-toggle="popover"]').popover();
		}
	    }
	
	loadCachedGraph(relativeIndex);
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
	sigmaNodes[i].count = parseInt(changedNodesArray[i].count);
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
    nodeSize = .125 * Math.pow(val, 1.5);
    sig.settings('maxNodeSize', nodeSize);
    sig.refresh({skipIndexation: true});
}

function arrowSizeChange(val) {
    arrowSize = val;
    sig.settings('minArrowSize', val);
    sig.refresh({skipIndexation: true});
}

function edgeSizeChange(val) {
    edgeSize = .00625 * Math.pow(val, 2);
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
    graphWidth = (21-val) / 10;
    sig.graph.nodes().forEach(function (n) {
	n.y = n.oy * graphWidth;
    });
    sig.refresh({skipIndexation: true});
}
