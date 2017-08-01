function Settings() {
    var defaultSettings = {

	// Inactive STEs:
	defaultSTEColor: "rgb(158,185,212)",
	defaultStartSTEColor: "rgb(100,100,100)",
	defaultReportSTEColor: "rgb(255,150,0)",
	defaultSpecialSTEColor: "rgb(255,0,255)",
	
	// Activated/Enabled STEs:
	reportingSTEColor: "rgb(255,0,255)",
	activatedSTEColor: "rgb(0,255,0)",
	enabledSTEColor: "rgb(255,255,0)"

    }
    this.settings = function() {
	return defaultSettings;
    }
}
