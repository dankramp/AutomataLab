function Settings() {
    var defaultSettings = {

	// Stage:
	stageBGColor: "#ffffff",                // rgb(255,255,255)
	selectedCharacterColor: "#ffff00",
	reportCharacterColor: "#ff77ff",
	selectAndReportColor: "#ffdd99",

	// Inactive STEs:
	defaultSTEColor: "#9eb9d4",            // rgb(158,185,212)
	defaultStartSTEColor: "#646464",       // rgb(100,100,100)
	defaultReportSTEColor: "#ff9600",      // rgb(255,150,0)
	defaultSpecialSTEColor: "#ff00ff",     // rgb(255,0,255)
	
	// Activated/Enabled STEs:
	reportingSTEColor: "#ff00ff",         // rgb(255,0,255)
	activatedSTEColor: "#00ff00",         // rgb(0,255,0)
	enabledSTEColor: "#ffff00",           // rgb(255,255,0)
	selectedSTEBorderColor: "#ffff77",    // rgb(255,255,119)

	// Edges:
	defaultEdgeColor: "#888888",          // rgb(136,136,136)
	edgeHoverColor: "#ff0000",            // rgb(255,0,0)

	// Editor Mode:
	editModeBGColor: "#ffffdd",            // rgb(255,255,238)
	lassoToolStrokeColor: "#ff0000"      // rgb(255,0,0)

    }
    this.settings = function() {
	return defaultSettings;
    }
}
