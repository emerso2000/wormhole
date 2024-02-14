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

int main() {
    vec3 cartesianVec(1.0f, 0.0f, 0.0f);
    vec3 newRayOrigin(1.0f, 1.0f, 1.0f);

    vec3 azElR = cartesianToAzELR(cartesianVec, newRayOrigin);
    vec3 cartesianResult = AzELRToCartesian(azElR, newRayOrigin);

    std::cout << "Original Cartesian Vec: (" << cartesianVec.x << ", " << cartesianVec.y << ", " << cartesianVec.z << ")" << std::endl;
    std::cout << "Converted AzELR Vec: (" << azElR.x << ", " << azElR.y << ", " << azElR.z << ")" << std::endl;
    std::cout << "Reconverted Cartesian Vec: (" << cartesianResult.x << ", " << cartesianResult.y << ", " << cartesianResult.z << ")" << std::endl;

    return 0;
}
