#include "graphics/flying_camera.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cmath>
#include <numbers>


flying_camera::flying_camera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& world_up) :
	camera(position, direction, world_up) {
}

void flying_camera::update(float deltaT, int dx, int dy) {

	static constexpr auto maxSpeed = 0.01f;
	static constexpr auto mouseSensitivity = 0.0001f;
	static constexpr auto friction = 1.0f - 0.6f;
	static constexpr auto pi = std::numbers::pi_v<float>;

	yaw += dx * mouseSensitivity * deltaT;
	pitch -= dy * mouseSensitivity * deltaT;

	yaw = std::fmod(yaw, 2.0f * pi);
	static constexpr float maxAngle = (pi / 2.0f) - std::numeric_limits<float>::epsilon();
	pitch = std::min(std::max(pitch, -maxAngle), maxAngle);

	glm::vec3 acceleration(0.f, 0.f, 0.f);

	using kb = sf::Keyboard;
	static const glm::vec3 noMove{ 0, 0, 0 };
	acceleration += kb::isKeyPressed(kb::W) ? front : kb::isKeyPressed(kb::S) ? -front : noMove;
	acceleration += kb::isKeyPressed(kb::D) ? right : kb::isKeyPressed(kb::A) ? -right : noMove;
	acceleration += kb::isKeyPressed(kb::Space) ? worldUp : kb::isKeyPressed(kb::LControl) ? -worldUp : noMove;

	if (glm::length(acceleration) > std::numeric_limits<float>::epsilon()) {
		velocity += glm::normalize(acceleration);
	}

	velocity *= friction;

	const float speed = glm::length(velocity);
	if (speed > maxSpeed) {
		velocity *= maxSpeed / speed;
	}

	const float speedBonus = kb::isKeyPressed(kb::LShift) ? 2.f : 1.f;
	position += velocity * speedBonus * deltaT;

	update_view_matrix();
}
