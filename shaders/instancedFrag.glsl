#version 450 core

out vec4 color; 

void main() {
    
    vec3 pointColor = vec3(1.0,1.0,1.0);
    vec2 uv = gl_PointCoord * 2.0 - 1.0;

    if(dot(uv,uv) > 1.0) {
        discard;
    }

    color = vec4(pointColor, 1.0);
}