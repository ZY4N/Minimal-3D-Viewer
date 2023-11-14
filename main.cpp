#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "geometry/mesh_loader.hpp"
#include "geometry/mesh.hpp"

#include <geometry/point_cloud.hpp>

#include <graphics/renderable_attributes.hpp>
#include <graphics/flying_camera.hpp>

#include <util/arx.hpp>
#include <geometry/point_cloud_loader.hpp>

#include "graphics/renderers/mesh_renderer.hpp"
#include "graphics/renderers/mesh_line_renderer.hpp"
#include "graphics/renderers/mesh_point_renderer.hpp"
#include "graphics/renderers/point_cloud_renderer.hpp"
#include <util/extra_arx_parsers.hpp>


using default_mesh = mesh<vertex_components::tex_coord, vertex_components::normal>;

using my_arx = ztu::arx<
	ztu::arx_flag<'f', "fullscreen">,
	ztu::arx_flag<'w', "m_width", int>,
	ztu::arx_flag<'h', "m_height", int>,
	ztu::arx_flag<'\0', "vsync">,
	ztu::arx_flag<'\0', "fps", unsigned int>,
	ztu::arx_flag<'\0', "spawn", glm::vec3, &extra_arx_parsers::glm_vec<3, float, glm::highp>>,
	ztu::arx_flag<'s', "size", glm::vec3, &extra_arx_parsers::glm_vec<3, float, glm::highp>>,
	ztu::arx_flag<'p', "pedantic">
>;

int main(int num_args, char* args[]) {

	//----------------------[ Argument Parsing ]----------------------//

	my_arx arguments(num_args, args);

	const auto fullscreen = arguments.get<"fullscreen">().value();
	const auto pedantic_enabled = arguments.get<"pedantic">().value();
	const auto vsync_enabled = arguments.get<"vsync">().value();
	const auto fps = arguments.get<"fps">().value_or(60);
	const auto spawn = arguments.get<"spawn">().value_or(glm::vec3{ 0, 0, 0 });
	const auto outer_box = arguments.get<"size">().value_or(glm::vec3{ 100, 100, 100 });

	constexpr auto title = "3D-Viewer";

	//----------------------[ Window/GLEW Setup ]----------------------//

	auto window = sf::RenderWindow(
		sf::VideoMode(1280, 720), title, sf::Style::Default, sf::ContextSettings(24, 8, 2, 4, 6)
	);
	auto [width, height] = window.getSize();

	if (glewInit() != GLEW_OK) {
		error<"Glew initialization failed, exiting.">();
		return -1;
	}

	sf::RectangleShape bar_background, bar_foreground;

	sf::Font font{};

	namespace fs = std::filesystem;

	//----------------------[ Loading Screen Setup ]----------------------//

	const auto current_path = fs::current_path();

	const auto font_file = current_path / ".." / "data" / "fonts" / "JetBrainsMono_Medium.ttf";
	if (not font.loadFromFile(font_file)) {
		error<"Could not open font file: %">(font_file);
	}

	const auto background_color = sf::Color{ 36, 41, 128 };

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(24);

	bar_background.setOutlineColor(sf::Color::Transparent);
	bar_foreground.setFillColor(sf::Color{ 16, 14, 35 });

	bar_background.setOutlineColor(sf::Color::Transparent);
	bar_background.setFillColor(sf::Color{ 21, 23, 59 });

	const auto shadow_offset = sf::Vector2f(5, 5);

	const auto set_progress = [&](const float progress, const char* status) {
		const auto bar_dim = sf::Vector2f(0.8f * float(width), 30.0f);
		const auto bar_pos = sf::Vector2f((float(width) - bar_dim.x) / 2.0f, (float(height) - bar_dim.y) / 2.0f);

		window.clear(background_color);

		bar_background.setSize(bar_dim);
		bar_background.setPosition(bar_pos + shadow_offset);
		window.draw(bar_background);

		bar_foreground.setSize(sf::Vector2f(progress * bar_dim.x, bar_dim.y));
		bar_foreground.setPosition(bar_pos);
		window.draw(bar_foreground);

		const auto percent = static_cast<int>(std::round(100.0f * progress));
		const auto num_digits = percent == 0 ? 0 : static_cast<int>(std::log10(percent));
		const auto status_percent = (
			std::string(status) +
				std::string(4 - num_digits, ' ') +
				std::to_string(percent) + '%'
		);
		text.setString(status_percent);
		const auto text_size = text.getGlobalBounds().getSize();
		const auto text_pos = sf::Vector2f(
			(float(width) - text_size.x) / 2.0f - shadow_offset.x,
			(float(height) - text_size.y) / 2.0f - shadow_offset.y
		);

		const auto text_offset = text_size.y / 15.0f;
		text.setPosition(text_pos + sf::Vector2f(text_offset, text_offset));
		text.setFillColor(sf::Color::Black);
		window.draw(text);

		text.setPosition(text_pos);
		text.setFillColor(sf::Color::White);
		window.draw(text);

		info<"state: %">(status);

		window.display();
	};

	set_progress(0, "Loading shaders");


	//----------------------[ Shader Setup ]----------------------//

	const auto shader_dir = std::filesystem::path{ "../shaders" };

	const auto shader_files = std::array{
		std::array{ fs::path{ "mesh_vertex.glsl" }, fs::path{ "" }, fs::path{ "mesh_fragment.glsl" } },
		std::array{ fs::path{ "mesh_line_vertex.glsl" }, fs::path{ "" }, fs::path{ "mesh_line_fragment.glsl" } },
		std::array{ fs::path{ "mesh_point_vertex.glsl" }, fs::path{ "" }, fs::path{ "mesh_point_fragment.glsl" } },
		std::array{ fs::path{ "point_vertex.glsl" }, fs::path{ "" }, fs::path{ "point_fragment.glsl" } }
	};
	using shader_tpl_t = std::tuple<shaders::meshes, shaders::mesh_lines, shaders::mesh_points, shaders::points>;
	shader_tpl_t shader_tpl;

	ztu::for_each::index<std::tuple_size_v<shader_tpl_t>>(
		[&]<auto Index>() {
			const auto& [vertex_file, geometry_file, fragment_file] = shader_files[Index];
			using shader_t = std::tuple_element_t<Index, shader_tpl_t>;
			shader_t& shader = std::get<Index>(shader_tpl);
			if (shader_t::from_files(
				shader_dir / vertex_file, shader_dir / geometry_file, shader_dir / fragment_file, shader
			)) {
				error<"Failed loading shader %">(vertex_file);
				exit(-1);
			}
			return false;
		}
	);


	[[maybe_unused]] auto m_mesh_renderer = mesh_renderer(&std::get<0>(shader_tpl));
	[[maybe_unused]] auto m_mesh_line_renderer = mesh_line_renderer(&std::get<1>(shader_tpl));
	[[maybe_unused]] auto m_mesh_point_renderer = mesh_point_renderer(&std::get<2>(shader_tpl));
	[[maybe_unused]] auto m_point_cloud_renderer = point_cloud_renderer(&std::get<3>(shader_tpl));

	//----------------------[ Asset loading ]----------------------//

	std::vector<default_mesh> meshes;
	std::unordered_map<std::string, std::shared_ptr<material>> materials;

	std::vector<basic_point_cloud> basic_point_clouds;
	std::vector<reflectance_point_cloud> reflectance_point_clouds;

	for (ztu::isize i = 0; i < arguments.num_positional(); i++) {
		auto path = fs::path{ arguments.get(i).value() };

		const auto progress = 0.2f + 0.4f * (
			static_cast<float>(i) / static_cast<float>(arguments.num_positional())
		);
		auto progress_title = std::string("Loading: ") + path.c_str();

		if (fs::is_directory(path)) {
			progress_title += " (3dtk)";
			set_progress(progress, progress_title.c_str());
			if (const auto e = point_cloud_loader::load_from_3dtk_directory(
					path, basic_point_clouds, reflectance_point_clouds
				); e) {
				warn<"Cannot parse directory %: %">(path, e.message());
			}
		} else if (path.extension() == ".c3d") {
			progress_title += " (compact 3dtk)";
			set_progress(progress, progress_title.c_str());
			std::vector<typename reflectance_point_cloud::vertex_t> points;
			if (const auto e = point_cloud_loader::load_v1_c3d_file(path, points); e) {
				warn<"Cannot read from %: %">(path, e.message());
			}
			reflectance_point_clouds.emplace_back(std::move(points));
		} else if (path.extension() == ".obj") {
			progress_title += " (Wavefront OBJ)";
			set_progress(progress, progress_title.c_str());
			if (const auto e = mesh_loader::load_from_obj(path, meshes, materials, pedantic_enabled); e) {
				info<"Cannot parse obj %: %">(path, e.message());
			}
		} else {
			warn<"Skipping %">(path);
			progress_title += " Skipped";
			set_progress(progress, progress_title.c_str());
		}
	}

	set_progress(0.6f, "Calculating model size");

	aabb model_box;
	ztu::u64 num_points = 0;
	for (const auto& point_cloud : basic_point_clouds) {
		model_box.join(point_cloud.calc_bounding_box());
		num_points += point_cloud.points().size();
	}
	for (const auto& point_cloud : reflectance_point_clouds) {
		model_box.join(point_cloud.calc_bounding_box());
		num_points += point_cloud.points().size();
	}
	debug<"num m_points: %">(num_points);

	ztu::u64 num_vertices = 0;
	for (const auto& mesh : meshes) {
		model_box.join(mesh.calc_bounding_box());
		num_vertices += mesh.vertex_buffer().size();
	}

	debug<"num m_vertices: %">(num_vertices);

	const auto model_size = model_box.size();
	debug<"model size: % % %">(model_size.x, model_size.y, model_size.z);


	const auto model_scale = std::min(
		{
			std::abs(model_size.x) < glm::epsilon<float>() ? 1 : (outer_box.x / model_size.x),
			std::abs(model_size.y) < glm::epsilon<float>() ? 1 : (outer_box.y / model_size.y),
			std::abs(model_size.z) < glm::epsilon<float>() ? 1 : (outer_box.z / model_size.z),
		}
	);

	const auto transform = glm::scale(
		glm::identity<glm::mat4x4>(),
		{ model_scale, model_scale, model_scale }
	);

	//----------------------[ Final OpenGL Context Initialization ]----------------------//

	set_progress(0.65f, "Initializing OpenGL");

	if (fullscreen) {
		window.create(sf::VideoMode(), title, sf::Style::Fullscreen, sf::ContextSettings(24, 8, 2, 4, 6));
		const auto new_size = window.getSize();
		width = new_size.x;
		height = new_size.y;
	}

	//glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
		[](
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam
		) {
			error<"[%] % type: %%, severity: %%, msg: %">(
				"OpenGL",
				(type == GL_DEBUG_TYPE_ERROR ? "ERROR" : ""),
				std::hex, type,
				std::hex, severity,
				message
			);
		},
		nullptr
	);


	info<"Rendering on %">(glGetString(GL_RENDERER));

	window.setVerticalSyncEnabled(vsync_enabled);
	window.setActive(true);

	//----------------------[ OpenGL Buffer Initialization ]----------------------//

	set_progress(0.7f, "Creating mesh instances");

	std::vector<mesh_instance> mesh_instances;
	mesh_instances.reserve(meshes.size());

	auto fallback_color_attr = std::make_shared<renderable_attributes::color>(glm::vec4(1, 0, 1, 1));
	auto fallback_point_size_attr = std::make_shared<renderable_attributes::point_size>(3.0f);

	for (auto& mesh : meshes) {
		mesh.init_vao();
		mesh_instances.push_back(mesh.create_instance(transform).value());
		bool found_color_attr = false;
		for (auto& attribute : mesh_instances.back().attributes) {
			if (attribute.index() == 0) {
				std::get<0>(attribute.attributes) = fallback_color_attr;
				found_color_attr = true;
			}
		}
		if (not found_color_attr) {
			mesh_instances.back().attributes.emplace_back(fallback_color_attr);
		}
		mesh_instances.back().attributes.emplace_back(fallback_point_size_attr);
	}

	set_progress(0.8f, "Creating point cloud instances");
	std::vector<point_cloud_instance> point_cloud_instances;
	point_cloud_instances.reserve(basic_point_clouds.size() + reflectance_point_clouds.size());

	for (auto& point_cloud : basic_point_clouds) {
		point_cloud.init_vao();
		point_cloud_instances.push_back(
			point_cloud.create_instance(transform).value()
		);
		point_cloud_instances.back().attributes.emplace_back(fallback_point_size_attr);
	}

	for (auto& point_cloud : reflectance_point_clouds) {
		point_cloud.init_vao();
		point_cloud_instances.push_back(
			point_cloud.create_instance(transform).value()
		);
		point_cloud_instances.back().attributes.emplace_back(fallback_point_size_attr);
	}

	std::array<std::shared_ptr<renderable_attributes::color>, 20> colors{};
	for (auto& attr_ptr : colors) {
		attr_ptr = std::make_shared<renderable_attributes::color>(rgba_colors::random());
	}

	for (auto i = 0; i < static_cast<ztu::isize>(point_cloud_instances.size()); i++) {
		point_cloud_instances[i].attributes.emplace_back(colors[i % 3]);
	}


	set_progress(1.0f, "Initialization complete");

	//----------------------[ final setup ]----------------------//

	auto scale = 1.0f;
	glm::mat4 proj_mat;
	const auto update_proj_mat = [&]() {
		static constexpr float halfPi = M_PI / 2.f;
		proj_mat = glm::perspective(
			halfPi / scale,
			float(width) / float(height),
			0.1f, 1000.0f
		);
	};

	update_proj_mat();

	debug<"Spawning at: % % %">(spawn.x, spawn.y, spawn.z);

	flying_camera player(spawn, { 0, 0, 1 }, { 0, 1, 0 });

	const auto frame_time = std::chrono::microseconds(int(1000000.0f / static_cast<float>(fps)));
	const auto dt = static_cast<float>(frame_time.count()) / 1000.f;

	bool running = true;
	bool lockMouse = true;
	window.setMouseCursorVisible(!lockMouse);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.f);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//----------------------[ Game Loop ]----------------------//

	while (running) {
		const auto start = std::chrono::high_resolution_clock::now();

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running = false;
			} else if (event.type == sf::Event::Resized) {
				glViewport(0, 0, event.size.width, event.size.height);
				width = event.size.width;
				height = event.size.height;
				update_proj_mat();
			} else if (event.type == sf::Event::MouseWheelMoved) {
				scale = std::max(scale * (1.0f + 0.1f * event.mouseWheel.delta), 1.f);
				update_proj_mat();
			} else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					running = false;
					break;
				case sf::Keyboard::Tab:
					window.setMouseCursorVisible(!(lockMouse ^= 1));
					break;
					//case sf::Keyboard::T: renderIndex = (renderIndex + 1) % renderers.size(); break;
				default:
					break;
				}
			}
		}

		if (lockMouse) [[likely]] {
			const int middleX = width / 2;
			const int middleY = height / 2;
			const auto mouseDelta = sf::Mouse::getPosition(window);
			sf::Mouse::setPosition({ middleX, middleY }, window);
			player.update(dt, mouseDelta.x - middleX, mouseDelta.y - middleY);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//renderers[renderIndex]->render(renderables, proj_mat, player.view_matrix());
		m_mesh_renderer.render(mesh_instances, proj_mat, player.view_matrix());
		m_point_cloud_renderer.render(point_cloud_instances, proj_mat, player.view_matrix());

		window.display();

		const auto finish = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(frame_time - (finish - start));
	}

	return 0;
}
