#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool lightOn;

void main()
{
    vec3 ambient = vec3(0.1f, 0.1f, 0.1f);
    
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = vec3(0.5f) * spec;

    if (lightOn)
    {
        FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        FragColor = vec4(ambient + diffuse + specular, 1.0f);
    }
}
