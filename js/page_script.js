function toggleChevron() {
    $('#nav-hide-icon').toggleClass("glyphicon glyphicon-chevron-up");
    $('#nav-hide-icon').toggleClass("glyphicon glyphicon-chevron-down");
}

function graph_tab_clicked() {
    // Deselect sim tab
    $('#sim-tab').removeClass("active");
    $('#sim-tools').hide();
    // Select graph tab
    $('#graph-tab').addClass("active");
    $('#graph-settings').show();
}

function sim_tab_clicked() {
    // Deselect graph tab
    $('#graph-tab').removeClass("active");
    $('#graph-settings').hide();
    // Select sim tab
    $('#sim-tab').addClass("active");
    $('#sim-tools').show();
}
