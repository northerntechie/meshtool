#version 130
in vec3 vPos;
in vec3 vNormal;
out vec3 oNormal;
out vec3 oPos;

uniform mat4 mM = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
                       0.0, 0.0, 0.0, 0.0, 1.0);
void main() {
  gl_Position = mM * vec4(vPos.x, vPos.y, vPos.z, 1.0);
  oPos = gl_Position.xyz;
	oNormal = (mM * vec4(vNormal, 1.0)).xyz;
}
