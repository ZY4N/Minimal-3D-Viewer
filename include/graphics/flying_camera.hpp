#pragma once

#include "graphics/camera.hpp"


class flying_camera : public camera {
public:
	flying_camera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& world_up);

	void update(float deltaT, int dx, int dy) override;
};
