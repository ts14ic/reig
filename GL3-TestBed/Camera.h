#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>

class Camera {
public:
	Camera(
		glm::vec3 const& position = defaultPosition(), // 0 0 0
		glm::vec3 const& worldUp = defaultUp(),	       // 0 1 0
		GLfloat const yaw = defaultYaw(),              // -90.0
		GLfloat const pitch = defaultPitch()           // 0.0
	);
	
	glm::mat4 const get_view() const;
	GLfloat const get_fov() const;
	GLfloat const get_zoom() const;
	
	void set_speed(GLfloat newSpeed = defaultSpeed()); // 5.f
	void set_fov(GLfloat newFOV = defaultFOV());       // 70.f
	
	enum Direction {
		forward,
		backward,
		up,
		down,
		left,
		right
	};
	
	void move(Direction direction, GLfloat deltaTime);
	void rotate(GLfloat xoffset, GLfloat yoffset);
	void zoom(GLfloat yoffset);
	
	static glm::vec3 const defaultPosition();
	static glm::vec3 const defaultUp();

	static GLfloat const defaultYaw();
	static GLfloat const defaultPitch();
	static GLfloat const defaultSpeed();
	static GLfloat const defaultSensivity();
	static GLfloat const defaultFOV();
	static GLfloat const maxFOV();
	static GLfloat const minFOV();

private:
	void _update_vectors();

	glm::vec3 _pos;
	glm::vec3 _worldUp;
	glm::vec3 _front;
	glm::vec3 _right;
	glm::vec3 _up;
	
	GLfloat _fov;
	GLfloat _yaw;
	GLfloat _pitch;
	
	GLfloat _speed;
	GLfloat _sensivity;
};

#endif // CAMERA_H_INCLUDED
