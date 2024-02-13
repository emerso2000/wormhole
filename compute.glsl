#version 460 core
#define PI = 3.14159265359

layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;

layout(std140, binding = 1) uniform CameraBlock {
    vec3 cam_origin;
    float padding1;
	vec3 forward;
    float padding2;
	vec3 right;
    float padding3;
	vec3 up;
	float fov;
} camera;

const int MAX_STEPS = 2000;
const float MAX_DIST = 100.0;
const float EPSILON = 0.01;


//point and radius
float sphereSDF(vec3 p, float r) {
    return length(p) - r;
}

vec3 marchRay(vec3 origin, vec3 direction) {
    float stepSize = 0.01;
    vec3 p = origin;

    for (int i = 0; i < MAX_STEPS; i++) {
        float distanceToSphere1 = sphereSDF(p, 0.5); //find distance to the sphere
        float distanceToSphere2 = sphereSDF(p - vec3(1.0, 0.0, 0.0), 0.5);
        float distanceToSphere3 = sphereSDF(p - vec3(-1.0, 0.0, 0.0), 0.5);

        float distanceToClosestSphere = min(min(distanceToSphere1, distanceToSphere2), distanceToSphere3);

        if (distanceToClosestSphere < EPSILON) {
            // Check which sphere is closer and return corresponding color
            if (distanceToClosestSphere == distanceToSphere1) {
                return vec3(1, 0, 0); // Red for the first sphere
            } 
            else if (distanceToClosestSphere == distanceToSphere2) {
                return vec3(0, 1, 0); // Green for the second sphere
            } 
            else {
                return vec3(0, 0, 1); // Blue for the third sphere
            }
        }

        if (distanceToClosestSphere > MAX_DIST) {
            return vec3(0.0); //if the distance is over the max distance, render black to save resources
        }
        p += direction * stepSize; //update the ray trajectory
    }
    return vec3(0, 0, 0); //return black if no hit
}

void main() {
    vec4 pixel = vec4(0.075, 0.133, 0.173, 1.0);
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    
    ivec2 dims = imageSize(screen);

    vec2 uv = (vec2(pixel_coords) - 0.5 * dims.xy) / dims.y;

    vec3 ray_origin = camera.cam_origin;
    //vec3 ray_direction = normalize(vec3(camera.right * uv.x + camera.up * uv.y + camera.forward * camera.fov)); 
    vec3 ray_direction = normalize(camera.forward * camera.fov + uv.x * camera.right + uv.y * camera.up); //directly incorporate the new rotated forward vector. 

    vec3 color = marchRay(ray_origin, ray_direction);

    pixel = vec4(color, 1.0);
    
    imageStore(screen, pixel_coords, pixel);
}