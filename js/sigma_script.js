
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
//var simIndex = 0;

function setCachedGraphs(json_object) {
    cachedGraphs = json_object;
    //console.log("cached: " + JSON.stringify(cachedGraphs));
}

function loadCachedGraph(index) {
    //console.log("index: " + index);
    var graph = cachedGraphs[0][index];
    //console.log("graph" + (cachedGraphs[0].length - index - 1) + ": " + JSON.stringify(graph));
    updateGraph(graph);
}

/*
 * For future use. Fully client-side simulation from cache
 *
$('#sim-step-btn').click(function (){
    simIndex++;
    loadCachedGraph(simIndex);
});
*/

function loadGraph(json_object){
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
	    n.count = 1;
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
    $('#reset-camera-btn').click(function(){
	sigma.misc.animation.camera(
	    sig.cameras[0],
	    { ratio: 1, x: 0, y: 0, angle: 0 },
	    { duration: 150 }
	);
    });
}




function updateGraph(updateJson) {
    //$('[data-toggle="tooltip"]').tooltip();
    // Revert the colors of all previously updated nodes and edges
    var timer = new Date().getTime();
    if (!heatMode) {
    changedGraph.nodes().forEach(function (n) {
	sig.graph.nodes(n.id).color = n.originalColor;
    });
    changedGraph.edges().forEach(function (e) {
	sig.graph.edges(e.id).color = "";
    });
	}
    changedGraph.clear();
    
    var changedNodesArray = updateJson.nodes;
    // Make an array of node IDs from incoming data
    var changedNodeIDs = [];
    for (var i = 0; i < changedNodesArray.length; i++)
	changedNodeIDs[i] = changedNodesArray[i].id;
    // Get array containing references to actual graph nodes
    var sigmaNodes = sig.graph.nodes(changedNodeIDs);
    if (!heatMode)
	// Add these nodes to changedGraph nodes
	for (var i = 0; i < sigmaNodes.length; i++)
	    changedGraph.addNode(sigmaNodes[i]);
    for (var i = 0; i < sigmaNodes.length; i++) {
	sigmaNodes[i].count++;
	if (heatMode) 
	    sigmaNodes[i].color = "rgb(" + toInt(255 - 255/sigmaNodes[i].count) + "," + Math.min(sigmaNodes[i].count-1, 255) + ",0)";
	else
	    sigmaNodes[i].color = changedNodesArray[i].color;
	// If node is activated, light up outgoing edges
	if (sigmaNodes[i].color == "rgb(0,255,0)" && !heatMode) {
	    var outgoingEdges = sig.graph.outEdges(sigmaNodes[i].id);
	    for (e in outgoingEdges) {
		outgoingEdges[e].color = "#0f0";
		changedGraph.addEdge(outgoingEdges[e]);
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
	    n.color = "rgb(" + toInt(255 - 255/n.count) + "," + Math.min(n.count-1, 255) + ",0)";
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
