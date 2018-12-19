#version 130
in vec3 oNormal;
in vec3 oPos;
out vec4 pColor;

uniform vec3 lightPos = normalize(vec3(0.0f,0.2f,-1.0f));

void main() {
  vec3 lightColor = vec3(0.8, 0.4, 0.55);
  // Ambient light source
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  // Diffuse light source
  vec3 norm = normalize(oNormal);
  vec3 lightDir = -lightPos;
  float diffuseStrength = max(dot(lightPos,norm),0.0);
  vec3 diffuse = diffuseStrength * lightColor;

  // Specular light glShaderSource
  vec3 viewPos = normalize(vec3(0.0,0.0,1.0));
  vec3 viewDir = normalize(viewPos - oPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float specularStrength = pow(max(dot(viewDir,reflectDir),0.0), 32);
  vec3 specular = specularStrength * lightColor;

  vec3 result = (ambient + diffuse + specular) * vec3(0.6, 0.6, 0.6);
  pColor = vec4(result, 1.0);
}
