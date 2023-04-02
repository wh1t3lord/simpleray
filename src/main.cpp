#include <iostream>
#include <fstream>

#include <glm/glm.hpp>

using namespace glm;

struct global_vars_t
{
};

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

/* init */
void init_window(global_vars_t& gvars) {}

void init(global_vars_t& gvars)
{
	init_window(gvars);
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

void update(global_vars_t& gvars)
{
	test_image(gvars);
	test_simple_ray(gvars);
	test_simple_sphere(gvars);
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
