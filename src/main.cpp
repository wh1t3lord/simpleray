#include <iostream>
#include <fstream>
#include <vector>
#include <variant>

#include <glm/glm.hpp>

using namespace glm;

struct global_vars_t;

constexpr double kInfinityDouble = std::numeric_limits<double>::infinity();
constexpr double kPI = 3.14159265358979323846;
constexpr glm::dvec3 kErrorColor = {224 / 255, 31 / 255, 199 / 255};

double math_convert_degrees_to_radians(double degrees)
{
	return degrees * kPI / 180.0;
}

double math_convert_radians_to_degrees(double radians)
{
	return radians * 180.0 / kPI;
}

#include <random>

// from 0.0 to 1.0
double math_random_double()
{
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}

/* image types */
class image_ppm_t
{
public:
	image_ppm_t() : m_is_opened{}, m_width{0}, m_height{0} {}
	image_ppm_t(int width, int height) :
		m_is_opened{}, m_width{width}, m_height{height}
	{
	}
	~image_ppm_t()
	{
		if (this->m_is_opened)
		{
			this->m_file.close();
		}
	}

	void write(const dvec3& color)
	{
		if (this->m_is_opened)
		{
			this->m_file << static_cast<int>(255.0 * color.x) << ' '
						 << static_cast<int>(255.0 * color.y) << ' '
						 << static_cast<int>(255.0 * color.z) << ' '
						 << std::endl;
		}
	}

	void write(const dvec3& color, int samples_per_pixel)
	{
		auto r = color.x;
		auto g = color.y;
		auto b = color.z;

		auto scale = 1.0 / samples_per_pixel;

		r *= scale;
		g *= scale;
		b *= scale;

		this->m_file << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
					 << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
					 << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
	}

	int get_width() const { return this->m_width; }
	int get_height() const { return this->m_height; }

	void set_width(int width) { this->m_width = width; }
	void set_height(int height) { this->m_height = height; }

	bool open(const char* p_file_name)
	{
		if (this->m_is_opened)
		{
			this->m_file.close();
			this->m_is_opened = false;
		}

		if (!p_file_name)
		{
			std::cout << "failed to open file because you passed invalid name"
					  << std::endl;
			return this->m_is_opened;
		}

		if (this->m_height && this->m_width)
		{
			this->m_file.open(p_file_name, std::ios::out);
			this->m_is_opened = this->m_file.good();

			if (this->m_is_opened)
			{
				this->m_file << "P3\n"
							 << this->m_width << ' ' << this->m_height
							 << "\n255\n";
			}
			else
			{
				this->m_is_opened = false;
				this->m_file.close();
			}

			return this->m_is_opened;
		}

		std::cout << "failed to open file because width or height is null"
				  << std::endl;

		return this->m_is_opened;
	}

	bool is_opened() const { return this->m_is_opened; }

private:
	bool m_is_opened;
	int m_width;
	int m_height;
	std::ofstream m_file;
};

/* math types */

/// @brief mathematical
class ray_t
{
public:
	ray_t() {}
	ray_t(const dvec3& origin, const dvec3& direction) :
		m_origin{origin}, m_direction{direction}
	{
	}

	~ray_t() {}

	const dvec3& get_origin() const { return this->m_origin; }
	dvec3& get_origin() { return this->m_origin; }

	const dvec3& get_direction() const { return this->m_direction; }
	dvec3& get_direction() { return this->m_direction; }

	dvec3 at(double t) const { return this->m_origin + t * this->m_direction; }

private:
	dvec3 m_origin;
	dvec3 m_direction;
};

class hit_record_t
{
public:
	hit_record_t() :
		m_is_hitted{}, m_is_front_face{},
		m_draw_normal_map{}, m_t{}, m_p_color{}
	{
	}
	~hit_record_t() {}

	bool is_front_face() const { return this->m_is_front_face; }
	void set_front_face(bool status) { this->m_is_front_face = status; }

	double get_t() const { return this->m_t; }
	void set_t(double value) { this->m_t = value; }

	const glm::dvec3& get_point(void) const { return this->m_point; }
	void set_point(const glm::dvec3& point) { this->m_point = point; }

	const glm::dvec3& get_normal(void) const { return this->m_normal; }
	void set_normal(const glm::dvec3& normal) { this->m_normal = normal; }

	bool is_hitted(void) const { return this->m_is_hitted; }
	void set_hitted(bool status) { this->m_is_hitted = status; }

	bool is_draw_normal_map() const { return this->m_draw_normal_map; }
	void set_draw_normal_map(bool status) { this->m_draw_normal_map = status; }

	const glm::dvec3* get_color() const { return this->m_p_color; }
	void set_color(const glm::dvec3* p_color) { this->m_p_color = p_color; }

private:
	bool m_is_hitted;
	bool m_is_front_face;
	bool m_draw_normal_map;
	double m_t;
	const glm::dvec3* m_p_color;
	glm::dvec3 m_point;
	glm::dvec3 m_normal;
};

enum class eEntityType : int
{
	kEntityType_Triangle = 1 << 0,
	kEntityType_Sphere = 1 << 1,
	kEntityType_Box = 1 << 2,
	kEntityType_Plane = 1 << 3,
	kEntityType_Pyramid = 1 << 4,
	kEntityType_Cone = 1 << 5,

	kEntityType_Unknown = -1
};

class sphere_data_t
{
public:
	sphere_data_t() {}
	sphere_data_t(bool draw_normal_map, double radius,
		const glm::dvec3& position, const glm::dvec3& color) :
		m_draw_normal_map{draw_normal_map},
		m_radius{radius}, m_position{position}, m_color{color}
	{
	}
	~sphere_data_t() {}

	double get_radius() const { return this->m_radius; }
	void set_radius(double value) { this->m_radius = value; }

	const glm::dvec3& get_position() const { return this->m_position; }
	void set_position(const glm::dvec3& position)
	{
		this->m_position = position;
	}

	bool is_draw_normal_map() const { return this->m_draw_normal_map; }
	void set_draw_normal_map(bool status) { this->m_draw_normal_map = status; }

	const glm::dvec3& get_color() const { return this->m_color; }
	void set_color(const glm::dvec3& color) { this->m_color = color; }

private:
	bool m_draw_normal_map;
	double m_radius;
	glm::dvec3 m_position;
	glm::dvec3 m_color;
};

class rectangle_data_t
{
public:
	rectangle_data_t() {}
	~rectangle_data_t() {}

private:
	glm::dvec3 m_poses[4];
};

class box_data_t
{
public:
	box_data_t() : m_radius{} {}
	~box_data_t() {}

	double get_radius() const { return this->m_radius; }
	void set_radius(double value) { this->m_radius = value; }

	const glm::dvec3& get_position() const { return this->m_position; }
	void set_position(const glm::dvec3& position)
	{
		this->m_position = position;
	}

private:
	double m_radius;
	glm::dvec3 m_position;
};

class entity_t
{
public:
	entity_t() : m_type{eEntityType::kEntityType_Unknown} {}
	entity_t(eEntityType type, const sphere_data_t& data) :
		m_type{type}, m_data{data}
	{
	}
	~entity_t() {}

	const sphere_data_t& get_sphere_data() const
	{
		return std::get<sphere_data_t>(this->m_data);
	}

	eEntityType get_type(void) const { return this->m_type; }
	void set_type(eEntityType type) { this->m_type = type; }

private:
	eEntityType m_type;
	std::variant<sphere_data_t> m_data;
};

class world_t
{
public:
	world_t() {}
	~world_t() {}

	void clear() { this->m_entities; }
	void add(const entity_t& object) { this->m_entities.push_back(object); }

	hit_record_t hit(
		const entity_t& entity, const ray_t& ray, double t_min, double t_max)
	{
		hit_record_t result;

		switch (entity.get_type())
		{
		case eEntityType::kEntityType_Triangle:
		{
			break;
		}
		case eEntityType::kEntityType_Sphere:
		{
			result = this->hit_sphere(entity, ray, t_min, t_max);
			break;
		}
		case eEntityType::kEntityType_Box:
		{
			break;
		}
		case eEntityType::kEntityType_Unknown:
		{
			break;
		}
		}

		return result;
	}

	const std::vector<entity_t>& get_entities() const
	{
		return this->m_entities;
	}

private:
	// in order to detect the sphere hit we need to solve this quadratic
	// equation (p(t) - c) * (p(t) - c) = r^2 where p(t) is our ray's formula a
	// + tb. So it goes like this (a + tb - c) * (a + tb - c) = r^2 and after
	// the final operations t^2⋅b⋅b + 2tb⋅(A−C) + (A−C)⋅(A−C)−r2=0 but for
	// better reading we re-write to general form of quadratic equation t^2 * a
	// + 2t * b + c = 0 so we need to define our a,b,c variables, but for sphere
	// we have b=2h situation that means we can reduce amount of computation,
	// because we just need half_b instead of squared b
	hit_record_t hit_sphere(
		const entity_t& entity, const ray_t& ray, double t_min, double t_max)
	{
		hit_record_t result;

		const auto& sphere_data = entity.get_sphere_data();

		auto oc = ray.get_origin() - sphere_data.get_position();

		// t^2*b*b or the a coefficient at t^2 in general form
		auto a = glm::dot(ray.get_direction(), ray.get_direction());

		// b * (A - C)
		auto half_b = glm::dot(oc, ray.get_direction());

		auto c = glm::dot(oc, oc) -
			sphere_data.get_radius() * sphere_data.get_radius();

		auto discriminant = half_b * half_b - a * c;

		if (discriminant < 0)
		{
			result.set_hitted(false);
			return result;
		}

		auto sqrtd = sqrt(discriminant);

		auto root = (-half_b - sqrtd) / a;

		if (root < t_min || t_max < root)
		{
			root = (-half_b + sqrtd) / a;

			if (root < t_min || t_max < root)
			{
				result.set_hitted(false);
				return result;
			}
		}

		result.set_t(root);
		result.set_point(ray.at(root));

		auto outward_normal =
			(result.get_point() - sphere_data.get_position()) /
			sphere_data.get_radius();

		if (glm::dot(outward_normal, ray.get_direction()) < 0)
		{
			result.set_normal(outward_normal);
			result.set_front_face(true);
		}
		else
		{
			result.set_normal(-outward_normal);
			result.set_front_face(false);
		}

		result.set_hitted(true);
		result.set_draw_normal_map(sphere_data.is_draw_normal_map());
		result.set_color(&sphere_data.get_color());
		return result;
	}

	hit_record_t hit_triangle(
		const entity_t& entity, const ray_t& ray, double t_min, double t_max)
	{
		hit_record_t result;

		return result;
	}

private:
	std::vector<entity_t> m_entities;
};

class camera_t
{
public:
	camera_t() {}
	camera_t(const glm::dvec3& origin, double aspect_ratio,
		double viewport_height, double focal_length = 1.0) :
		m_origin{origin}
	{
		double viewport_width = aspect_ratio * viewport_height;

		this->m_horizontal = glm::dvec3(viewport_width, 0.0, 0.0);
		this->m_vertical = glm::dvec3(0.0, viewport_height, 0.0);

		this->m_lower_left_corner = this->m_origin -
			(this->m_horizontal / 2.0) - (this->m_vertical / 2.0) -
			glm::dvec3(0.0, 0.0, focal_length);
	}
	~camera_t() {}

	const glm::dvec3& get_origin() const { return this->m_origin; }
	void set_origin(const glm::dvec3& origin) { this->m_origin = origin; }

	const glm::dvec3& get_lower_left_corner() const
	{
		return this->m_lower_left_corner;
	}
	void set_lower_left_corner(const glm::dvec3& coord)
	{
		this->m_lower_left_corner = coord;
	}

	const glm::dvec3& get_horizontal() const { return this->m_horizontal; }
	void set_horizontal(const glm::dvec3& coord) { this->m_horizontal = coord; }

	const glm::dvec3& get_vertical() const { return this->m_vertical; }
	void set_vertical(const glm::dvec3& coord) { this->m_vertical = coord; }

	ray_t get_ray(double u, double v)
	{
		return ray_t(this->m_origin,
			this->m_lower_left_corner + u * this->m_horizontal +
				v * this->m_vertical - this->m_origin);
	}

private:
	glm::dvec3 m_origin;
	glm::dvec3 m_lower_left_corner;
	glm::dvec3 m_horizontal;
	glm::dvec3 m_vertical;
};

struct global_vars_t
{
	global_vars_t() : m_samples_per_pixel{} {}
	~global_vars_t() {}

	int m_samples_per_pixel;
	camera_t m_camera;
};

/* init */
void init_window(global_vars_t& gvars) {}

void init(global_vars_t& gvars)
{
	init_window(gvars);
}

/* draw functions */

// just linear interpolation between two colors
glm::dvec3 draw_gradient(
	double value, const glm::dvec3& from, const glm::dvec3& to)
{
	return (1.0 - value) * from + value * to;
}

// normalizing result of normal + color because color RGB components can't be
// higher than 1.0 for that case we multiply on 0.5
glm::dvec3 draw_normal(const glm::dvec3& normal)
{
	return 0.5 * (normal + glm::dvec3(1.0, 1.0, 1.0));
}

/* simulation */

bool hit_sphere(const glm::dvec3& center, double radius, const ray_t& ray)
{
	auto oc = ray.get_origin() - center;

	auto a = glm::dot(ray.get_direction(), ray.get_direction());
	auto b = 2.0 * glm::dot(oc, ray.get_direction());
	auto c = glm::dot(oc, oc) - radius * radius;

	auto discriminant = (b * b) - (4 * a * c);

	return (discriminant > 0);
}

void test_image(global_vars_t& gvars)
{
	std::cout << "writing test image" << std::endl;

	image_ppm_t img(256, 256);

	auto status = img.open("test1_gradient.ppm");

	if (status)
	{
		auto total_size = img.get_width() * img.get_height();
		for (int i = 1; i <= total_size; ++i)
		{
			img.write({double(i) / total_size, double(i) / total_size,
				double(i) / total_size});
		}
	}

	std::cout << "test.ppm was created" << std::endl;
}

void test_simple_ray(global_vars_t& gvars)
{
	auto aspect_ratio = 16.0 / 9.0;
	auto width = 400;
	auto height = width / aspect_ratio;

	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = glm::dvec3(0, 0, 0);

	auto horizontal = glm::dvec3(viewport_width, 0, 0);
	auto vertical = glm::dvec3(0, viewport_height, 0);
	auto lower_left_corner = origin - (horizontal / 2.0) - (vertical / 2.0) -
		glm::dvec3(0, 0, focal_length);

	image_ppm_t img(width, height);

	img.open("test2_background.ppm");

	for (int j = height - 1; j >= 0; --j)
	{
		for (int i = 0; i < width; ++i)
		{
			auto u = double(i) / (width - 1);
			auto v = double(j) / (height - 1);

			ray_t ray(origin,
				lower_left_corner + (u * horizontal) + (v * vertical) - origin);

			auto norm_dir = glm::normalize(ray.get_direction());

			auto t = 0.5 * (norm_dir.y + 1.0);

			auto color = (1.0 - t) * glm::dvec3(1.0, 1.0, 1.0) +
				t * glm::dvec3(0.5, 0.7, 1.0);

			img.write(color);
		}
	}
}

void test_simple_sphere(global_vars_t& gvars)
{
	auto aspect_ratio = 16.0 / 9.0;
	auto width = 400;
	auto height = width / aspect_ratio;

	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = glm::dvec3(0, 0, 0);

	auto horizontal = glm::dvec3(viewport_width, 0, 0);
	auto vertical = glm::dvec3(0, viewport_height, 0);
	auto lower_left_corner = origin - (horizontal / 2.0) - (vertical / 2.0) -
		glm::dvec3(0, 0, focal_length);

	image_ppm_t img(width, height);

	img.open("test3_sphere.ppm");

	for (int j = height - 1; j >= 0; --j)
	{
		for (int i = 0; i < width; ++i)
		{
			auto u = double(i) / (width - 1);
			auto v = double(j) / (height - 1);

			ray_t ray(origin,
				lower_left_corner + (u * horizontal) + (v * vertical) - origin);

			glm::dvec3 color;
			if (hit_sphere({0, 0, -1}, 0.5, ray))
			{
				color = {1, 0, 0};
			}
			else
			{
				auto norm_dir = glm::normalize(ray.get_direction());

				auto t = 0.5 * (norm_dir.y + 1.0);

				color = (1.0 - t) * glm::dvec3(1.0, 1.0, 1.0) +
					t * glm::dvec3(0.5, 0.7, 1.0);
			}

			img.write(color);
		}
	}
}

void test_world_sphere(global_vars_t& gvars)
{
	auto aspect_ratio = 16.0 / 9.0;
	auto width = 400;
	auto height = width / aspect_ratio;

	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = glm::dvec3(0, 0, 0);

	auto horizontal = glm::dvec3(viewport_width, 0, 0);
	auto vertical = glm::dvec3(0, viewport_height, 0);
	auto lower_left_corner = origin - (horizontal / 2.0) - (vertical / 2.0) -
		glm::dvec3(0, 0, focal_length);

	world_t world;
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(true, 0.5, {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0})));

	image_ppm_t img(width, height);

	img.open("test4_world_sphere.ppm");

	constexpr double inf = std::numeric_limits<double>::infinity();

	for (int j = height - 1; j >= 0; --j)
	{
		for (int i = 0; i < width; ++i)
		{
			auto u = double(i) / (width - 1);
			auto v = double(j) / (height - 1);

			ray_t ray(origin,
				lower_left_corner + (u * horizontal) + (v * vertical) - origin);

			// it works only for one entity
			for (const auto& entity : world.get_entities())
			{
				const auto& hit_result = world.hit(entity, ray, 0.0, inf);

				if (hit_result.is_hitted())
				{
					if (hit_result.is_draw_normal_map())
					{
						img.write(draw_normal(hit_result.get_normal()));
					}
				}
				else
				{
					auto t =
						0.5 * (glm::normalize(ray.get_direction()).y + 1.0);
					img.write(
						draw_gradient(t, {1.0, 1.0, 1.0}, {0.5, 0.7, 1.0}));
				}
			}
		}
	}
}

void test_world_sphere_with_ground(global_vars_t& gvars)
{
	auto aspect_ratio = 16.0 / 9.0;
	auto width = 400;
	auto height = width / aspect_ratio;

	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = glm::dvec3(0, 0, 0);

	auto horizontal = glm::dvec3(viewport_width, 0, 0);
	auto vertical = glm::dvec3(0, viewport_height, 0);
	auto lower_left_corner = origin - (horizontal / 2.0) - (vertical / 2.0) -
		glm::dvec3(0, 0, focal_length);

	world_t world;
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(false, 100.0, {0.0, -100.8, -1.0}, {0.0, 1.0, 0.0})));
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(true, 0.5, {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0})));

	image_ppm_t img(width, height);

	img.open("test5_world_sphere_with_ground.ppm");

	glm::dvec3 output_color;

	for (int j = height - 1; j >= 0; --j)
	{
		for (int i = 0; i < width; ++i)
		{
			auto u = double(i) / (width - 1);
			auto v = double(j) / (height - 1);

			ray_t ray(origin,
				lower_left_corner + (u * horizontal) + (v * vertical) - origin);

			bool is_hitted{};

			for (const auto& entity : world.get_entities())
			{
				const auto& hit_result =
					world.hit(entity, ray, 0.0, kInfinityDouble);

				if (hit_result.is_hitted())
				{
					is_hitted = true;
					if (hit_result.is_draw_normal_map())
					{
						output_color = draw_normal(hit_result.get_normal());
					}
					else
					{
						if (hit_result.get_color())
						{
							output_color = *hit_result.get_color();
						}
						else
						{
							output_color = kErrorColor;
						}
					}
				}
			}

			if (!is_hitted)
			{
				auto t = 0.5 * (glm::normalize(ray.get_direction()).y + 1.0);
				output_color =
					draw_gradient(t, {1.0, 1.0, 1.0}, {0.5, 0.7, 1.0});
			}

			img.write(output_color);
		}
	}
}

void test_world_sphere_with_ground_new_aspect_ratio(global_vars_t& gvars)
{
	auto aspect_ratio = 4.0 / 3.0;
	auto width = 400;
	auto height = width / aspect_ratio;

	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;

	auto origin = glm::dvec3(0, 0, 0);

	auto horizontal = glm::dvec3(viewport_width, 0, 0);
	auto vertical = glm::dvec3(0, viewport_height, 0);
	auto lower_left_corner = origin - (horizontal / 2.0) - (vertical / 2.0) -
		glm::dvec3(0, 0, focal_length);

	world_t world;
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(false, 100.0, {0.0, -100.8, -1.0}, {0.0, 1.0, 0.0})));
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(true, 0.5, {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0})));

	image_ppm_t img(width, height);

	img.open("test5_world_sphere_with_ground_new_ratio.ppm");

	glm::dvec3 output_color;

	for (int j = height - 1; j >= 0; --j)
	{
		for (int i = 0; i < width; ++i)
		{
			auto u = double(i) / (width - 1);
			auto v = double(j) / (height - 1);

			ray_t ray(origin,
				lower_left_corner + (u * horizontal) + (v * vertical) - origin);

			bool is_hitted{};

			for (const auto& entity : world.get_entities())
			{
				const auto& hit_result =
					world.hit(entity, ray, 0.0, kInfinityDouble);

				if (hit_result.is_hitted())
				{
					is_hitted = true;
					if (hit_result.is_draw_normal_map())
					{
						output_color = draw_normal(hit_result.get_normal());
					}
					else
					{
						if (hit_result.get_color())
						{
							output_color = *hit_result.get_color();
						}
						else
						{
							output_color = kErrorColor;
						}
					}
				}
			}

			if (!is_hitted)
			{
				auto t = 0.5 * (glm::normalize(ray.get_direction()).y + 1.0);
				output_color =
					draw_gradient(t, {1.0, 1.0, 1.0}, {0.5, 0.7, 1.0});
			}

			img.write(output_color);
		}
	}
}

void test_world_camera(global_vars_t& gvars)
{
	auto aspect_ratio = 16.0 / 9.0;
	auto width = 400;
	auto height = width / aspect_ratio;
	auto viewport_height = 2.0;
	gvars.m_camera = camera_t({0.0, 0.0, 0.0}, aspect_ratio, viewport_height);
	gvars.m_samples_per_pixel = 100;

	world_t world;
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(false, 100.0, {0.0, -100.8, -1.0}, {0.0, 1.0, 0.0})));
	world.add(entity_t(eEntityType::kEntityType_Sphere,
		sphere_data_t(true, 0.5, {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0})));

	image_ppm_t img(width, height);

	img.open("test6_world_camera.ppm");

	for (int j = height - 1; j >= 0; --j)
	{
		for (int i = 0; i < width; ++i)
		{
			glm::dvec3 output_color;

			for (int sample_index = 0; sample_index < gvars.m_samples_per_pixel;
				 ++sample_index)
			{
				auto u = (double(i) + math_random_double()) / (width - 1);
				auto v = (double(j) + math_random_double()) / (height - 1);

				const auto& ray = gvars.m_camera.get_ray(u, v);

				bool is_hitted{};

				for (const auto& entity : world.get_entities())
				{
					const auto& hit_result =
						world.hit(entity, ray, 0.0, kInfinityDouble);

					if (hit_result.is_hitted())
					{
						is_hitted = true;
						if (hit_result.is_draw_normal_map())
						{
							output_color +=
								draw_normal(hit_result.get_normal());
						}
						else
						{
							if (hit_result.get_color())
							{
								output_color += *hit_result.get_color();
							}
							else
							{
								output_color += kErrorColor;
							}
						}
					}
				}

				if (!is_hitted)
				{
					auto t =
						0.5 * (glm::normalize(ray.get_direction()).y + 1.0);
					output_color +=
						draw_gradient(t, {1.0, 1.0, 1.0}, {0.5, 0.7, 1.0});
				}
			}

			img.write(output_color, gvars.m_samples_per_pixel);
		}
	}
}

void update(global_vars_t& gvars)
{
	test_image(gvars);
	test_simple_ray(gvars);
	test_simple_sphere(gvars);
	test_world_sphere(gvars);
	test_world_sphere_with_ground(gvars);
	test_world_sphere_with_ground_new_aspect_ratio(gvars);
	test_world_camera(gvars);
}

/* deinit */
void deinit_window(global_vars_t& gvars) {}

void deinit(global_vars_t& gvars)
{
	deinit_window(gvars);
}

int main()
{
	global_vars_t gvars;

	init(gvars);

	update(gvars);

	deinit(gvars);

	return 0;
}
