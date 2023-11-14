#include "graphics/camera.hpp"
#include <glm/gtx/transform.hpp>
#include <cmath>


camera::camera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& worldUp) :
	position{ position }, view_direction{ direction }, worldUp{ worldUp } {
	matrix = glm::mat4(1.f);
	velocity = glm::vec3(0.f, 0.f, 0.f);

	right = glm::vec3(0.f);
	up = worldUp;

	pitch = 0.f;
	yaw = glm::radians(-90.f);
	roll = 0.f;

	update_view_matrix();
}

void camera::update_view_matrix() {
	front.x = std::cos(yaw) * std::cos(pitch);
	front.y = std::sin(pitch);
	front.z = std::sin(yaw) * std::cos(pitch);

	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	matrix = glm::lookAt(position, position + front, up);
}

const glm::mat4& camera::view_matrix() {
	return matrix;
}
