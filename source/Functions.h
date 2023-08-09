#pragma once
#include "glm/glm.hpp"
#include "types.h"
#include <vector>
#include <string>

namespace Functions {
	template<class T>
	T nan() { return std::numeric_limits<T>::quiet_NaN(); }
	template<class T>
	T min() { return std::numeric_limits<T>::min(); }
	template<class T>
	T max() { return std::numeric_limits<T>::max(); }

	//String
	bool equal(std::string& s1, std::string& s2);
	bool equal(std::string& s1, char* s2);
	bool equal(std::string& s1, const char* s2);
	bool equal(char* s1, std::string& s2);
	bool equal(const char* s1, std::string& s2);
	//END_String

	bool equal(float x, float y, float eps);

	float linearStep(float x, glm::vec2 p0, glm::vec2 p1);
	float percentile(std::vector<float> collection, float percent);
	float distance(glm::vec2 p0, glm::vec2 p1);
	glm::vec3 lineProjection(glm::vec3 v, glm::vec3 d_hat);
	glm::vec3 planeProjection(glm::vec3 v, glm::vec3 d_hat);
	
	Quad perspective(Quad q, glm::vec3 eye, Plane plane);
	glm::mat4 affineTransform(Quad qi, Quad qf);

	float triangleArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	glm::vec3 barycentricCoordinates(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p);
	bool inXY(Quad q, glm::vec2 p);
	bool in(Quad q, glm::vec3 p);

	Pixels border(Pixels p);
	float quadraticRadialBasis(float d, float min, float max, float percent);

	float pixelDistance(glm::uvec2 p0, glm::uvec2 p1);
	float linear(float L, float R, float u);
	float bilinear(float B_L, float B_R, float T_L, float T_R, float u, float v);

	glm::vec3 solve(glm::mat3 M, glm::vec3 b);
	glm::vec4 solve(glm::mat4 M, glm::vec4 b);

	float trimmedAverage(std::vector<float> values, float min_percentile, float max_percentile);
	
	template<class T, typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
	T ceilingMultiple(T v, T mult) {
		if (v % mult == 0) { return v; }
		return v + mult - (v % mult);
	}

	template<class T, typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
    T floorMultiple(T v, T mult) {
        return v - (v % mult);
    }

	template<class T, typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
    T nearestMultiple(T v, T mult) {
        T floorM = floorMultiple(v, mult);
        if (2 * (v - floorM) < mult) return floorM;
        return floorM + mult;
    }
}
