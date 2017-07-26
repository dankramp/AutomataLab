function toggleChevron() {
    $('#nav-hide-icon').toggleClass("glyphicon glyphicon-chevron-up");
    $('#nav-hide-icon').toggleClass("glyphicon glyphicon-chevron-down");
}

function graph_tab_clicked() {
    // Deselect other tabs
    $('#sim-tab').removeClass("active");
    $('#editor-tab').removeClass("active");
    $('#sim-tools').hide();
    $('#editor-tools').hide();
    // Select graph tab
    $('#graph-tab').addClass("active");
    $('#graph-settings').show();
}

function sim_tab_clicked() {
    // Deselect other tabs
    $('#graph-tab').removeClass("active");
    $('#editor-tab').removeClass("active");
    $('#graph-settings').hide();
    $('#editor-tools').hide();
    // Select sim tab
    $('#sim-tab').addClass("active");
    $('#sim-tools').show();
}

function editor_tab_clicked() {
    // Deselect other tabs
    $('#graph-tab').removeClass("active");
    $('#sim-tab').removeClass("active");
    $('#graph-settings').hide();
    $('#sim-tools').hide();
    // Select editor tab
    $('#editor-tab').addClass("active");
    $('#editor-tools').show();
}
