;(function(undefined) {
  'use strict';

  if (typeof sigma === 'undefined')
    throw 'sigma is not declared';

  // Initialize packages:
  sigma.utils.pkg('sigma.canvas.hovers');

  /**
   * This hover renderer will basically display the label with a background.
   *
   * @param  {object}                   node     The node object.
   * @param  {CanvasRenderingContext2D} context  The canvas context.
   * @param  {configurable}             settings The settings function.
   */
  sigma.canvas.hovers.def = function(node, context, settings) {
    var x,
        y,
        w,
        h,
        e,
        phi,
        i,
        text,
        fontStyle = settings('hoverFontStyle') || settings('fontStyle'),
        prefix = settings('prefix') || '',
        size = node[prefix + 'size'],
        fontSize = (settings('labelSize') === 'fixed') ?
          settings('defaultLabelSize') :
          settings('labelSizeRatio') * size;

    // Label background:
    context.font = (fontStyle ? fontStyle + ' ' : '') +
      fontSize + 'px ' + (settings('hoverFont') || settings('font'));

    context.beginPath();
    context.fillStyle = settings('labelHoverBGColor') === 'node' ?
      (node.color || settings('defaultNodeColor')) :
      settings('defaultHoverLabelBGColor');

    if (node.label && settings('labelHoverShadow')) {
      context.shadowOffsetX = 0;
      context.shadowOffsetY = 0;
      context.shadowBlur = 8;
      context.shadowColor = settings('labelHoverShadowColor');
    }

    if (node.label && typeof node.label === 'string') {
      text = node.label.split("\n");
      w = 0;
      for (i = 0; i < text.length; i++)
        w = Math.max(w, Math.round(
	  context.measureText(text[i]).width + fontSize / 2 + 8
	));
	
      x = Math.round(node[prefix + 'x']);
      y = Math.round(node[prefix + 'y']);
      h = Math.round(fontSize * text.length + 4 * (text.length + 1));
      e = Math.round(node[prefix + 'size'] * 1.2);
      phi = Math.PI - 2 * Math.acos(e / Math.sqrt(.25 * w * w + e * e));

 	context.beginPath();     
	context.moveTo(x, y + e);
	context.arc(x, y, e, .5 * Math.PI, .5 * Math.PI + phi);
	context.lineTo(x - .5 * w, y - e);
	context.lineTo(x - .5 * w, y - h - e);
	context.lineTo(x + .5 * w, y - h - e);
	context.lineTo(x + .5 * w, y - e);
	context.lineTo(x + Math.sin(phi) * e, y + Math.cos(phi) * e);
	context.arc(x, y, e, .5 * Math.PI - phi, .5 * Math.PI);
	context.stroke(); 

      context.closePath();
      context.fill();

      context.shadowOffsetX = 0;
      context.shadowOffsetY = 0;
      context.shadowBlur = 0;
    }

    // Node border:
    if (settings('borderSize') > 0) {
      context.beginPath();
      context.fillStyle = settings('nodeBorderColor') === 'node' ?
        (node.color || settings('defaultNodeColor')) :
        settings('defaultNodeBorderColor');
      context.arc(
        node[prefix + 'x'],
        node[prefix + 'y'],
        size + settings('borderSize'),
        0,
        Math.PI * 2,
        true
      );
      context.closePath();
      context.fill();
    }

    // Node:
    var nodeRenderer = sigma.canvas.nodes[node.type] || sigma.canvas.nodes.def;
    nodeRenderer(node, context, settings);

    // Display the label:
    if (node.label && typeof node.label === 'string') {
      context.fillStyle = (settings('labelHoverColor') === 'node') ?
        (node.color || settings('defaultNodeColor')) :
        settings('defaultLabelHoverColor');
	for (var i = 0; i < text.length; i++) {
	    context.fillText(
		text[i],
		Math.round(node[prefix + 'x'] - w * .5 + 4),
		Math.round(node[prefix + 'y'] - h - e + (4 + fontSize) * (i + 1))
	    );
	}
    }
  };
}).call(this);
