#version 130
attribute vec3 vPos;
attribute vec3 vColor;
attribute vec3 vNormal;
varying vec4 oColor;
varying vec3 oNormal;
uniform mat4 mM = mat4(1.0, 0.0, 0.0, 0.0,
		       0.0, 1.0, 0.0, 0.0,
		       0.0, 0.0, 1.0, 0.0,
		       0.0, 0.0, 0.0, 1.0);
void main() {
  oNormal = vNormal;
  oColor = vec4(vPos, 1.0);
  gl_Position = mM * vec4(vPos.x, vPos.y, vPos.z, 1.0);
}
