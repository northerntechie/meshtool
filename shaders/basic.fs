#version 130
in vec4 oColor;
in vec3 oNormal;
out vec4 pColor;

uniform vec3 eye = vec3(0.0, 0.8, -1.0);

void main() {
  float intensity = dot(normalize(eye),oColor.xyz);
  intensity = max(intensity, 0.0);
  pColor = vec4(intensity, intensity, intensity, 1.0);
}
