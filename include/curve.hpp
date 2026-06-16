#ifndef CURVE_HPP__
#define CURVE_HPP__

#include <glm/glm.hpp>

class Curves{
public:
	std::vector<glm::vec3> points;
	float currentT = 0.0f;
	float speed = 0.5f;

	Curves(std::vector<glm::vec3> points) : points(points){
	}

	void update(float deltaTime) {
		currentT += speed * deltaTime;

		if (currentT > 1.0f) {
			currentT = 0.0f;
		}

		currentT = glm::clamp(currentT, 0.0f, 1.0f);
	}

	glm::vec3 getPosition() {

		if (points.empty()) {
			return glm::vec3(0.0f);
		}

		if (points.size() == 1) {
			return points[0];
		}

		// De Casteljau's algorithm
		std::vector<glm::vec3> temp = points;
		int n = points.size() - 1;

		for (int r = 1; r <= n; r++) {
			for (int i = 0; i <= n - r; i++) {
				temp[i] = (1.0f - currentT) * temp[i] + currentT * temp[i + 1];
			}
		}

		return temp[0];
	}
};

std::vector<glm::vec3> points = {
	glm::vec3(0.0f, 0.0f, 0.0f),      // Start at origin
	glm::vec3(0.0f, 5.0f, 5.0f),      // Move up and forward
	glm::vec3(10.0f, 5.0f, 5.0f),     // Move right while staying up/forward
	glm::vec3(10.0f, 0.0f, 10.0f)     // End at different Z
};
Curves CURVE_TEST(points);

#endif
