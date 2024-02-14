#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

vec3 cartesianToAzELR(vec3 cartesianVec, vec3 newRayOrigin) {
    float r = newRayOrigin.x;
    float th = newRayOrigin.y;
    float phi = newRayOrigin.z;

    mat3 transformationMatrix = mat3(
        sin(th) * cos(phi), sin(th) * sin(phi), cos(th),
        cos(th) * cos(phi), cos(th) * sin(phi), -sin(th),
        -sin(phi), cos(phi), 0.0
    );

    vec3 newVec = transformationMatrix * cartesianVec;

    return newVec;
}

vec3 AzELRToCartesian(vec3 sphericalVec, vec3 newRayOrigin) {
    float r = newRayOrigin.x;
    float th = newRayOrigin.y;
    float phi = newRayOrigin.z;

    mat3 transformationMatrix = mat3(
        sin(th) * cos(phi), cos(th) * cos(phi), -sin(phi),
        sin(th) * sin(phi), cos(th) * sin(phi), cos(phi),
        cos(th), -sin(th), 0.0
    );

    vec3 newVec = transformationMatrix * sphericalVec;

    return newVec;
}

vec3 cartesianToSpherical(vec3 cartesian) {
    float radius = length(cartesian);
    float theta = acos(cartesian.z / radius);
    float phi = atan(cartesian.y, cartesian.x);

    return vec3(radius, theta, phi);
}

vec3 sphericalToCartesian(vec3 spherical) {
    float x = spherical.x * sin(spherical.y) * cos(spherical.z);
    float y = spherical.x * sin(spherical.y) * sin(spherical.z);
    float z = spherical.x * cos(spherical.y);

    return vec3(x, y, z);
}


int main() {
    vec3 cartesianVec(1.0f, 2.0f, 0.0f);
    vec3 newRayOrigin(1.0f, 1.0f, 1.0f);

    vec3 azElR = cartesianToAzELR(cartesianVec, newRayOrigin);
    vec3 cartesianResult = AzELRToCartesian(azElR, newRayOrigin);

    std::cout << "Original Cartesian Vec: (" << cartesianVec.x << ", " << cartesianVec.y << ", " << cartesianVec.z << ")" << std::endl;
    std::cout << "Converted AzELR Vec: (" << azElR.x << ", " << azElR.y << ", " << azElR.z << ")" << std::endl;
    std::cout << "Reconverted Cartesian Vec: (" << cartesianResult.x << ", " << cartesianResult.y << ", " << cartesianResult.z << ")" << std::endl;

    vec3 sphericalVec = cartesianToSpherical(cartesianVec);
    vec3 cartesianResult2 = sphericalToCartesian(sphericalVec);

    std::cout << "\n Cartesian Vec: (" << cartesianVec.x << ", " << cartesianVec.y << ", " << cartesianVec.z << ")" << std::endl;
    std::cout << "Converted Spherical Vec: (" << sphericalVec.x << ", " << sphericalVec.y << ", " << sphericalVec.z << ")" << std::endl;
    std::cout << "Reconverted Cartesian Vec: (" << cartesianResult2.x << ", " << cartesianResult2.y << ", " << cartesianResult2.z << ")" << std::endl;

    return 0;
}
