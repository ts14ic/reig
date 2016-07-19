#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

glm::vec3 const Camera::defaultPosition() {
	return {0.f, 0.f, 0.f};
}

glm::vec3 const Camera::defaultUp() {
	return {0.f, 1.f, 0.f};
}

GLfloat const Camera::defaultYaw() {
	return -90.f;
}
GLfloat const Camera::defaultPitch() {
	return 0.f;
}
GLfloat const Camera::defaultSpeed() {
	return 5.f;
}
GLfloat const Camera::defaultSensivity() {
	return 0.05f;
}
GLfloat const Camera::maxFOV() {
	return 70.f;
}
GLfloat const Camera::minFOV() {
	return 15.f;
}

Camera::Camera(
	glm::vec3 const& position,
	glm::vec3 const& worldUp,
	GLfloat const    yaw,
	GLfloat const    pitch
)
: _pos{position}, _worldUp{worldUp}, _yaw{yaw}, _pitch{pitch}
{
	_speed     = defaultSpeed();
	_sensivity = defaultSensivity();
	_fov       = maxFOV();
	_update_vectors();
}

glm::mat4 const Camera::get_view() const {
	return glm::lookAt(_pos, _pos + _front, _up);
}

void Camera::_update_vectors() {
	_front.x = std::cos(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
	_front.y = std::sin(glm::radians(_pitch));
	_front.z = std::sin(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
	_front = glm::normalize(_front);
	
	_right = glm::normalize(glm::cross(_front, _worldUp));
	_up    = glm::normalize(glm::cross(_right, _front));
}

void Camera::set_speed(GLfloat newSpeed) {
	_speed = newSpeed;
}

void Camera::set_fov(GLfloat newFOV) {
	_fov = newFOV;
	if(_fov > 70.f) _fov = 70.f;
	else if(_fov < 15.f) _fov = 15.f;
}

void Camera::move(Direction direction, GLfloat deltaTime) {
	GLfloat moveBy = _speed * deltaTime;
	
	switch(direction) {
		case forward:
		_pos += _front * moveBy;
		break;
		
		case backward:
		_pos -= _front * moveBy;
		break;
		
		case up:
		_pos += _up * moveBy;
		break;
		
		case down:
		_pos -= _up * moveBy;
		break;
		
		case right:
		_pos += _right * moveBy;
		break;
		
		case left:
		_pos -= _right * moveBy;
		break;
	}
}
	
void Camera::rotate(GLfloat xoffset, GLfloat yoffset) {
	xoffset *= _sensivity;
	yoffset *= _sensivity;
	
	_yaw += xoffset;
	_pitch += yoffset;
	
	if(_pitch > 89.f) _pitch = 89.f;
	else if(_pitch < -89.f) _pitch = -89.f;
	
	_update_vectors();
}

void Camera::zoom(GLfloat yoffset) {
	if(yoffset < 0 && _fov <= 70.f) _fov -= yoffset * 4.f;
	else if(yoffset > 0 && _fov >= 15.f) _fov -= yoffset * 4.f;
}

GLfloat const Camera::get_fov() const {
	return _fov;
}

GLfloat const Camera::get_zoom() const {
	return _fov;
}
