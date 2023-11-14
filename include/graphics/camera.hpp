#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>


class camera {
public:
	camera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& worldUp);

	void update_view_matrix();

	const glm::mat4& view_matrix();

	virtual void update(float deltaT, int dx, int dy) = 0;

private:
	glm::mat4 matrix;

protected:
	glm::vec3 position, velocity, view_direction;
	glm::vec3 worldUp, front, right, up;

	float pitch, yaw, roll;

};
