;(function() {
  'use strict';

  sigma.utils.pkg('sigma.webgl.nodes');

  /**
   * This node renderer will display nodes as discs, shaped in triangles with
   * the gl.TRIANGLES display mode. So, to be more precise, to draw one node,
   * it will store three times the center of node, with the color and the size,
   * and an angle indicating which "corner" of the triangle to draw.
   *
   * The fragment shader does not deal with anti-aliasing, so make sure that
   * you deal with it somewhere else in the code (by default, the WebGL
   * renderer will oversample the rendering through the webglOversamplingRatio
   * value).
   */
  sigma.webgl.nodes.square = {
    POINTS: 6,
    ATTRIBUTES: 5,
    addNode: function(node, data, i, prefix, settings) {
      var color = sigma.utils.floatColor(
        node.color || settings('defaultNodeColor')
      );

      data[i++] = node[prefix + 'x'];
      data[i++] = node[prefix + 'y'];
      data[i++] = node[prefix + 'size'];
      data[i++] = color;
      data[i++] = Math.PI / 4;

      data[i++] = node[prefix + 'x'];
      data[i++] = node[prefix + 'y'];
      data[i++] = node[prefix + 'size'];
      data[i++] = color;
      data[i++] = 3 * Math.PI / 4;

      data[i++] = node[prefix + 'x'];
      data[i++] = node[prefix + 'y'];
      data[i++] = node[prefix + 'size'];
      data[i++] = color;
      data[i++] = 5 * Math.PI / 4;

      data[i++] = node[prefix + 'x'];
      data[i++] = node[prefix + 'y'];
      data[i++] = node[prefix + 'size'];
      data[i++] = color;
      data[i++] = 5 * Math.PI / 4;

      data[i++] = node[prefix + 'x'];
      data[i++] = node[prefix + 'y'];
      data[i++] = node[prefix + 'size'];
      data[i++] = color;
      data[i++] = 7 * Math.PI / 4;

      data[i++] = node[prefix + 'x'];
      data[i++] = node[prefix + 'y'];
      data[i++] = node[prefix + 'size'];
      data[i++] = color;
      data[i++] = Math.PI / 4;
    },
    render: function(gl, program, data, params) {
      var buffer;

      // Define attributes:
      var positionLocation =
            gl.getAttribLocation(program, 'a_position'),
          sizeLocation =
            gl.getAttribLocation(program, 'a_size'),
          colorLocation =
            gl.getAttribLocation(program, 'a_color'),
          angleLocation =
            gl.getAttribLocation(program, 'a_angle'),
          resolutionLocation =
            gl.getUniformLocation(program, 'u_resolution'),
          matrixLocation =
            gl.getUniformLocation(program, 'u_matrix'),
          ratioLocation =
            gl.getUniformLocation(program, 'u_ratio'),
          scaleLocation =
            gl.getUniformLocation(program, 'u_scale');

      buffer = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
      gl.bufferData(gl.ARRAY_BUFFER, data, gl.DYNAMIC_DRAW);

      gl.uniform2f(resolutionLocation, params.width, params.height);
      gl.uniform1f(
        ratioLocation,
        1 / Math.pow(params.ratio, params.settings('nodesPowRatio'))
      );
      gl.uniform1f(scaleLocation, params.scalingRatio);
      gl.uniformMatrix3fv(matrixLocation, false, params.matrix);

      gl.enableVertexAttribArray(positionLocation);
      gl.enableVertexAttribArray(sizeLocation);
      gl.enableVertexAttribArray(colorLocation);
      gl.enableVertexAttribArray(angleLocation);

      gl.vertexAttribPointer(
        positionLocation,
        2,
        gl.FLOAT,
        false,
        this.ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT,
        0
      );
      gl.vertexAttribPointer(
        sizeLocation,
        1,
        gl.FLOAT,
        false,
        this.ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT,
        8
      );
      gl.vertexAttribPointer(
        colorLocation,
        1,
        gl.FLOAT,
        false,
        this.ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT,
        12
      );
      gl.vertexAttribPointer(
        angleLocation,
        1,
        gl.FLOAT,
        false,
        this.ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT,
        16
      );

      gl.drawArrays(
        gl.TRIANGLES,
        params.start || 0,
        params.count || (data.length / this.ATTRIBUTES)
      );
    },
    initProgram: function(gl) {
      var vertexShader,
          fragmentShader,
          program;

      vertexShader = sigma.utils.loadShader(
        gl,
        [
          'attribute vec2 a_position;',
          'attribute float a_size;',
          'attribute float a_color;',
          'attribute float a_angle;',

          'uniform vec2 u_resolution;',
          'uniform float u_ratio;',
          'uniform float u_scale;',
          'uniform mat3 u_matrix;',

          'varying vec4 color;',
          'varying float radius;',

          'void main() {',
            // Multiply the point size twice:
            'radius = a_size * u_ratio;',

            // Scale from [[-1 1] [-1 1]] to the container:
            'vec2 position = (u_matrix * vec3(a_position, 1)).xy;',

            'position = position +',
              '1.5 * radius * vec2(cos(a_angle), sin(a_angle));',
            'position = (position / u_resolution * 2.0 - 1.0) * vec2(1, -1);',

            'radius = radius * u_scale;',

            'gl_Position = vec4(position, 0, 1);',

            // Extract the color:
            'float c = a_color;',
            'color.b = mod(c, 256.0); c = floor(c / 256.0);',
            'color.g = mod(c, 256.0); c = floor(c / 256.0);',
            'color.r = mod(c, 256.0); c = floor(c / 256.0); color /= 255.0;',
            'color.a = 1.0;',
          '}'
        ].join('\n'),
        gl.VERTEX_SHADER
      );

      fragmentShader = sigma.utils.loadShader(
        gl,
        [
          'precision mediump float;',

          'varying vec4 color;',

          'void main(void) {',
            'gl_FragColor = color;',
          '}'
        ].join('\n'),
        gl.FRAGMENT_SHADER
      );

      program = sigma.utils.loadProgram(gl, [vertexShader, fragmentShader]);

      return program;
    }
  };
})();
