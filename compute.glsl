#version 460 core
#define PI 3.14159265359

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

float atan2(in float y, in float x) {
    return x == 0.0 ? sign(y)*PI/2 : atan(y, x);
}

vec3 cartesianToSpherical(vec3 cartesian) {
    float radius = length(cartesian);
    float theta = acos(cartesian.y / radius);
    float phi = atan2(cartesian.z, cartesian.x);

    return vec3(radius, theta, phi);
}

vec3 sphericalToCartesian(vec3 spherical) {
    float x = spherical.x * sin(spherical.y) * cos(spherical.z);
    float y = spherical.x * cos(spherical.y);
    float z = spherical.x * sin(spherical.y) * sin(spherical.z);

    return vec3(x, y, z);
}

vec3 cartesianToAzELR(vec3 cartesianVec, vec3 newRayOrigin) {
    float r = newRayOrigin.x;
    float th = newRayOrigin.y;
    float phi = newRayOrigin.z;

    mat3 transformationMatrix = mat3(
        sin(th)*cos(phi),  sin(th)*sin(phi),  cos(th),
        cos(th)*cos(phi),  cos(th)*sin(phi), -sin(th),
        -sin(phi),  cos(phi),  0
    );
    
    vec3 newVec = transformationMatrix * cartesianVec;

    //return newVec.yzx;
    return newVec;
}

mat3 calculateChristoffelSymbolsAlphaR(vec3 position) {
    float r = position.x;
    float theta = position.y;

    mat3 christoffelSymbols_alpha_r;

    float rs = 0.0; // Schwarzschild radius

    christoffelSymbols_alpha_r[0][0] = -rs /((2.0 * r) * (r - rs));
    christoffelSymbols_alpha_r[0][1] = 0.0;
    christoffelSymbols_alpha_r[0][2] = 0.0;

    christoffelSymbols_alpha_r[1][0] = 0.0;
    christoffelSymbols_alpha_r[1][1] = rs - r;
    christoffelSymbols_alpha_r[1][2] = 0.0;

    christoffelSymbols_alpha_r[2][0] = 0.0;
    christoffelSymbols_alpha_r[2][1] = 0.0;
    christoffelSymbols_alpha_r[2][2] = (rs - r) * sin(theta) * sin(theta);

    return christoffelSymbols_alpha_r;
}

mat3 calculateChristoffelSymbolsAlphaTheta(vec3 position) {
    float r = position.x;
    float theta = position.y;

    mat3 christoffelSymbols_alpha_theta;

    float rs = 0.0; // Schwarzschild radius

    christoffelSymbols_alpha_theta[0][0] = 0.0;
    christoffelSymbols_alpha_theta[0][1] = 1.0 / r;
    christoffelSymbols_alpha_theta[0][2] = 0.0;

    christoffelSymbols_alpha_theta[1][0] = 1.0 / r;
    christoffelSymbols_alpha_theta[1][1] = 0.0;
    christoffelSymbols_alpha_theta[1][2] = 0.0;

    christoffelSymbols_alpha_theta[2][0] = 0.0;
    christoffelSymbols_alpha_theta[2][1] = 0.0;
    christoffelSymbols_alpha_theta[2][2] = -sin(theta) * cos(theta);

    return christoffelSymbols_alpha_theta;
}

mat3 calculateChristoffelSymbolsAlphaPhi(vec3 position) {
    float r = position.x;
    float theta = position.y;

    mat3 christoffelSymbols_alpha_phi;

    float rs = 0.0; // Schwarzschild radius

    christoffelSymbols_alpha_phi[0][0] = 0.0;
    christoffelSymbols_alpha_phi[0][1] = 0.0;
    christoffelSymbols_alpha_phi[0][2] = 1.0 / r;

    christoffelSymbols_alpha_phi[1][0] = 0.0;
    christoffelSymbols_alpha_phi[1][1] = 0.0;
    christoffelSymbols_alpha_phi[1][2] = 1.0 / tan(theta);

    christoffelSymbols_alpha_phi[2][0] = 1.0 / r;
    christoffelSymbols_alpha_phi[2][1] = 1.0 / tan(theta);
    christoffelSymbols_alpha_phi[2][2] = 0.0;

    return christoffelSymbols_alpha_phi;
}

vec3 marchRay(vec3 origin, vec3 direction) {
    float stepSize = 0.01;
    vec3 p = origin;

    vec3 accel = vec3(0.0);

    for (int i = 0; i < MAX_STEPS; i++) {
        p += direction * stepSize; //update the ray trajectory
        int wall = 8;
        vec3 p_cart = sphericalToCartesian(p);

        if (p.x < 0.7) {
            return vec3(1.0, 0.0, 0.0); //sphere
        }
        if (p_cart.z <= -wall) {
            return vec3(0.0, 1.0, 0.0); //first wall
        }
        if (p_cart.x >= wall) {
            return vec3(0.0, 0.0, 1.0); // Second wall
        }
        if (p_cart.x <= -wall) {
            return vec3(1.0, 1.0, 0.0); // Third wall
        }
        if (p_cart.z >= wall) {
            return vec3(0.0, 1.0, 1.0); // Wall behind the camera
        }
        if (p_cart.y <= -wall) {
            return vec3(0.5, 0.0, 0.0); //floor
        }
        if (p_cart.y >= wall) {
            return vec3(0.5, 0.5, 0.0); //ceiling
        }

        mat3 christoffelSymbols_alpha_r = calculateChristoffelSymbolsAlphaR(p);
        mat3 christoffelSymbols_alpha_theta = calculateChristoffelSymbolsAlphaTheta(p);
        mat3 christoffelSymbols_alpha_phi = calculateChristoffelSymbolsAlphaPhi(p);

        // Calculate the accelerations using the geodesic equation
        accel.x = -dot(direction, christoffelSymbols_alpha_r * direction);
        accel.y = -dot(direction, christoffelSymbols_alpha_theta * direction);
        accel.z = -dot(direction, christoffelSymbols_alpha_phi * direction);

        direction += accel * stepSize;
    }
    return vec3(0.115, 0.133, 0.173);
}

void main() {
    vec4 pixel = vec4(0.115, 0.133, 0.173, 1.0);
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    
    ivec2 dims = imageSize(screen);

    vec2 uv = (vec2(pixel_coords) - 0.5 * dims.xy) / dims.y;

    vec3 ray_origin = camera.cam_origin;
    vec3 ray_direction = normalize(camera.forward * camera.fov + uv.x * camera.right + uv.y * camera.up); //directly incorporate the new rotated forward vector. 

    vec3 sphericalRayOrigin = cartesianToSpherical(ray_origin);
    vec3 sphericalRayDirection = cartesianToAzELR(ray_direction, sphericalRayOrigin);

    sphericalRayDirection.y /= sphericalRayOrigin.x;
    sphericalRayDirection.z /= (sphericalRayOrigin.x * sin(sphericalRayOrigin.y));

    vec3 color = marchRay(sphericalRayOrigin, sphericalRayDirection);

    pixel = vec4(color, 1.0);
    
    imageStore(screen, pixel_coords, pixel);
}
