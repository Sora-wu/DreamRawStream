#version 460 core

uniform sampler2D textureY;
uniform sampler2D textureU;
uniform sampler2D textureV;

in vec2 uv;
out vec4 fragColor;

void main() {
    vec3 yuv;
    yuv.x = texture(textureY, uv).r;
    yuv.y = texture(textureU, uv).r - 0.5;
    yuv.z = texture(textureV, uv).r - 0.5;
    vec3 rgb = mat3( 1,1,1, 0,-0.39465,2.03211,1.13983,-0.58060,0) * yuv;
    fragColor = vec4(rgb, 1);
}
