#include "Shader.hpp"
#include <GL/glew.h>
#include <fstream>
#include <iostream>

Shader::Shader(int type)
    : _id{glCreateShader(type)} 
{
}

Shader::Shader(int type, char const* src) 
    : Shader{type}
{
    load_source(src);
}

Shader::Shader(int type, char const* path, ShaderFromFile)
    : Shader{type}
{
    load_source(path, ShaderFromFile {});
}

Shader::~Shader() {
    glDeleteShader(_id);
}

int Shader::get() const {
    return _id;
}

int Shader::load_source(char const* src) {
    glShaderSource(_id, 1, &src, nullptr);
    glCompileShader(_id);
    GLint status; 
    glGetShaderiv(_id, GL_COMPILE_STATUS, &status);
    return status;
}

int Shader::load_source(char const* path, ShaderFromFile) {
    std::ifstream file {path};
    
    std::string src;
    for(std::string line; std::getline(file, line); src += line + '\n');
    
    char const* csrc = src.c_str();
    glShaderSource(_id, 1, &csrc, nullptr);
    glCompileShader(_id);
    GLint status; 
    glGetShaderiv(_id, GL_COMPILE_STATUS, &status);
    return status;
}

bool Shader::ready() const {
    GLint ret;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &ret);
    return ret == GL_TRUE;
}

std::string Shader::get_log() const {
    char buffer[250];
    glGetShaderInfoLog(_id, sizeof buffer, nullptr, buffer);
    return std::string{buffer};
}

ShaderProgram::ShaderProgram(Shader const& vertex, Shader const& fragment)  {
    attach(vertex, fragment);
}

ShaderProgram::~ShaderProgram() {
    GLint inUse;
    glGetIntegerv(GL_CURRENT_PROGRAM, &inUse);
    if(_id == static_cast<unsigned>(inUse)) {
        unuse();
    }
    glDeleteProgram(_id);
}

int ShaderProgram::get() const {
    return _id;
}

int ShaderProgram::uniform(char const* name) const {
    return glGetUniformLocation(_id, name);
}

void ShaderProgram::use() const {
    glUseProgram(_id);
}

void ShaderProgram::unuse() {
    glUseProgram(0);
}

int ShaderProgram::attach(Shader const& vertex, Shader const& fragment) {
    _id = glCreateProgram();
    glAttachShader(_id, vertex.get());
    glAttachShader(_id, fragment.get());
    glLinkProgram(_id);
    GLint ret;
    glGetProgramiv(_id, GL_LINK_STATUS, &ret);
    return ret;
}

bool ShaderProgram::ready() const {
    GLint status;
    glGetProgramiv(_id, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

std::string ShaderProgram::get_log() const {
    char buffer[500];
    glGetProgramInfoLog(_id, sizeof buffer, nullptr, buffer);
    return std::string{buffer};
}
