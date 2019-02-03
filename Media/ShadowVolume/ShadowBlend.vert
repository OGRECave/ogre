uniform mat4 worldViewProj;
attribute vec4 vertex;

void main() {
    gl_Position = worldViewProj*vertex;
}
