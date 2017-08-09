/**
 * Sigma Rectangle Selector
 * Modified version of Lasso
 * =============================
 *
 * @author Dan Kramp
 * @version 1.0
 */
;(function (undefined) {
  'use strict';

  if (typeof sigma === 'undefined')
    throw 'sigma is not declared';

  // Initialize package:
  sigma.utils.pkg('sigma.plugins');

   var _body = undefined,
       _instances = {};

  /**
   * RectSelect Object
   * ------------------
   * @param  {sigma}                                  sigmaInstance The related sigma instance.
   * @param  {renderer} renderer                      The sigma instance renderer.
   * @param  {sigma.classes.configurable} settings    A settings class
   */
  function RectSelect (sigmaInstance, renderer, settings) {
    // RectSelect is also an event dispatcher
    sigma.classes.dispatcher.extend(this);

    // A quick hardcoded rule to prevent people from using this plugin with the
    // WebGL renderer (which is impossible at the moment):
    if (
      sigma.renderers.webgl &&
      renderer instanceof sigma.renderers.webgl
    )
      throw new Error(
        'The sigma.plugins.rectSelect is not compatible with the WebGL renderer'
      );

    this.sigmaInstance = sigmaInstance;
    this.renderer = renderer;
    this.drawingCanvas = undefined;
    this.drawingContext = undefined;
    this.sourcePoint = {};
    this.endPoint = {};
    this.selectedNodes = [];
    this.isActive = false;
    this.isDrawing = false;

    _body = document.body;

    // Extends default settings
    this.settings = new sigma.classes.configurable({
      'strokeStyle': 'rgb(0,102,204)',
      'lineWidth': 1,
      'fillWhileDrawing': true,
      'fillStyle': 'rgba(0, 120, 215, 0.25)',
      'cursor': 'crosshair'
     }, settings || {});
  };

  /**
   * This method is used to destroy the rectangle selector.
   *
   * > var rect = new sigma.plugins.rectSelect(sigmaInstance);
   * > rect.clear();
   *
   * @return {sigma.plugins.rectSelect} Returns the instance.
   */
  RectSelect.prototype.clear = function () {
    this.stop();

    this.sigmaInstance = undefined;
    this.renderer = undefined;

    return this;
  };

  // RectSelect.prototype.getSigmaInstance = function () {
  //   return this.sigmaInstance;
  // }

  /**
   * This method is used to start the rectSelect mode.
   *
   * > var rect = new sigma.plugins.rectSelect(sigmaInstance);
   * > rect.start();
   *
   * @return {sigma.plugins.rectSelect} Returns the instance.
   */
  RectSelect.prototype.start = function () {
    if (this.sigmaInstance && !this.isActive) {
      this.isActive = true;

      // Add a new background layout canvas to draw the path on
      if (!this.renderer.domElements['rectSelect']) {
        this.renderer.initDOM('canvas', 'rectSelect');
        this.drawingCanvas = this.renderer.domElements['rectSelect'];

        this.drawingCanvas.width = this.renderer.container.offsetWidth;
        this.drawingCanvas.height = this.renderer.container.offsetHeight;
        this.renderer.container.appendChild(this.drawingCanvas);
        this.drawingContext = this.drawingCanvas.getContext('2d');
        this.drawingCanvas.style.cursor = this.settings('cursor');
      }

      _bindAll.apply(this);
    }

    return this;
  };

  /**
   * This method is used to stop the rectSelect mode.
   *
   * > var rect = new sigma.plugins.rectSelect(sigmaInstance);
   * > rect.stop();
   *
   * @return {sigma.plugins.rectSelect} Returns the instance.
   */
  RectSelect.prototype.stop = function () {
    if (this.sigmaInstance && this.isActive) {
      this.isActive = false;
      this.isDrawing = false;

      _unbindAll.apply(this);

      if (this.renderer.domElements['rectSelect']) {
        this.renderer.container.removeChild(this.renderer.domElements['rectSelect']);
        delete this.renderer.domElements['rectSelect'];
        this.drawingCanvas.style.cursor = '';
        this.drawingCanvas = undefined;
        this.drawingContext = undefined;
        this.drewPoints = [];
      }
    }

    return this;
  };

  /**
   * This method is used to bind all events of the rectSelect mode.
   *
   * > var rect = new sigma.plugins.rectSelect(sigmaInstance);
   * > rect.start();
   *
   * @return {sigma.plugins.rectSelect} Returns the instance.
   */
  var _bindAll = function () {
    // Mouse events
    this.drawingCanvas.addEventListener('mousedown', onDrawingStart.bind(this));
    _body.addEventListener('mousemove', onDrawing.bind(this));
    _body.addEventListener('mouseup', onDrawingEnd.bind(this));
    // Touch events
    this.drawingCanvas.addEventListener('touchstart', onDrawingStart.bind(this));
    _body.addEventListener('touchmove', onDrawing.bind(this));
    _body.addEventListener('touchcancel', onDrawingEnd.bind(this));
    _body.addEventListener('touchleave', onDrawingEnd.bind(this));
    _body.addEventListener('touchend', onDrawingEnd.bind(this));
  };

  /**
   * This method is used to unbind all events of the rectSelect mode.
   *
   * > var rect = new sigma.plugins.rectSelect(sigmaInstance);
   * > rect.stop();
   *
   * @return {sigma.plugins.rectSelect} Returns the instance.
   */
  var _unbindAll = function () {
    // Mouse events
    this.drawingCanvas.removeEventListener('mousedown', onDrawingStart.bind(this));
    _body.removeEventListener('mousemove', onDrawing.bind(this));
    _body.removeEventListener('mouseup', onDrawingEnd.bind(this));
    // Touch events
    this.drawingCanvas.removeEventListener('touchstart', onDrawingStart.bind(this));
    this.drawingCanvas.removeEventListener('touchmove', onDrawing.bind(this));
    _body.removeEventListener('touchcancel', onDrawingEnd.bind(this));
    _body.removeEventListener('touchleave', onDrawingEnd.bind(this));
    _body.removeEventListener('touchend', onDrawingEnd.bind(this));
  };

  /**
   * This method is used to retrieve the previously selected nodes
   *
   * > var rect = new sigma.plugins.rectSelect(sigmaInstance);
   * > rect.getSelectedNodes();
   *
   * @return {array} Returns an array of nodes.
   */
  RectSelect.prototype.getSelectedNodes = function () {
    return this.selectedNodes;
  };

  function onDrawingStart (event) {
    var drawingRectangle = this.drawingCanvas.getBoundingClientRect(),
      x = 0,
      y = 0;

    if (this.isActive) {
      this.isDrawing = true;
      this.selectedNodes = [];

      this.sigmaInstance.refresh();

      switch (event.type) {
        case 'touchstart':
          x = event.touches[0].clientX;
          y = event.touches[0].clientY;
          break;
        default:
          x = event.clientX;
          y = event.clientY;
          break;
      }

      this.sourcePoint = {
        x: x - drawingRectangle.left,
        y: y - drawingRectangle.top
      };

      this.drawingCanvas.style.cursor = this.settings('cursor');

      event.stopPropagation();
    }
  }

  function onDrawing (event) {
    if (this.isActive && this.isDrawing) {
      var x = 0,
          y = 0,
          drawingRectangle = this.drawingCanvas.getBoundingClientRect();
      switch (event.type) {
        case 'touchmove':
          x = event.touches[0].clientX;
          y = event.touches[0].clientY;
          break;
        default:
          x = event.clientX;
          y = event.clientY;
          break;
      }
      this.endPoint = {
        x: x - drawingRectangle.left,
        y: y - drawingRectangle.top
      };

      // Drawing styles
      this.drawingContext.lineWidth = this.settings('lineWidth');
      this.drawingContext.strokeStyle = this.settings('strokeStyle');
      this.drawingContext.fillStyle = this.settings('fillStyle');
      this.drawingContext.lineJoin = 'round';
      this.drawingContext.lineCap = 'round';
      this.drawingContext.setLineDash([5, 3]);

      // Clear the canvas
      this.drawingContext.clearRect(0, 0, this.drawingContext.canvas.width, this.drawingContext.canvas.height);

      // Draw the rectangle
      this.drawingContext.beginPath();
      this.drawingContext.moveTo(this.sourcePoint.x, this.sourcePoint.y);
      this.drawingContext.rect(this.sourcePoint.x, this.sourcePoint.y, this.endPoint.x - this.sourcePoint.x, this.endPoint.y - this.sourcePoint.y);
      this.drawingContext.stroke();

      if (this.settings('fillWhileDrawing')) {
        this.drawingContext.fill();
      }

      event.stopPropagation();
    }
  }

  function onDrawingEnd (event) {
    if (this.isActive && this.isDrawing) {
      this.isDrawing = false;

      // Select the nodes inside the path
      var nodes = this.renderer.nodesOnScreen,
        nodesLength = nodes.length,
        i = 0,
        prefix = this.renderer.options.prefix || '';

      // Loop on all nodes and check if they are in the path
      while (nodesLength--) {
        var node = nodes[nodesLength],
            x = node[prefix + 'x'],
            y = node[prefix + 'y'];

        if (this.drawingContext.isPointInPath(x, y) && !node.hidden) {
          this.selectedNodes.push(node);
        }
      }

      // Dispatch event with selected nodes
      this.dispatchEvent('selectedNodes', this.selectedNodes);

      // Clear the drawing canvas
      this.drawingContext.clearRect(0, 0, this.drawingCanvas.width, this.drawingCanvas.height);

      this.drawingCanvas.style.cursor = this.settings('cursor');

      event.stopPropagation();
	this.stop();
    }
  }

  /**
   * @param  {sigma}                                  sigmaInstance The related sigma instance.
   * @param  {renderer} renderer                      The sigma instance renderer.
   * @param  {sigma.classes.configurable} settings    A settings class
   *
   * @return {sigma.plugins.rectSelect} Returns the instance
   */
  sigma.plugins.rectSelect = function (sigmaInstance, renderer, settings) {
    // Create rectSelect if undefined
    if (!_instances[sigmaInstance.id]) {
      _instances[sigmaInstance.id] = new RectSelect(sigmaInstance, renderer, settings);
    }

    // Listen for sigmaInstance kill event, and remove the rectSelect instance
    sigmaInstance.bind('kill', function () {
      if (_instances[sigmaInstance.id] instanceof RectSelect) {
        _instances[sigmaInstance.id].clear();
        delete _instances[sigmaInstance.id];
      }
    });

    return _instances[sigmaInstance.id];
  };

    sigma.plugins.killRect = function(s) {
	if (_instances[s.id] instanceof RectSelect) {
            _instances[s.id].clear();
            delete _instances[s.id];
	}
    };

}).call(this);
