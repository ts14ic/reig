#ifndef SHADER_HPP_INCLUDED
#define SHADER_HPP_INCLUDED

#include <string>

struct ShaderFromFile {};

class Shader {
public:
    explicit Shader(int type);
    Shader(int type, char const* src);
    Shader(int type, char const* path, ShaderFromFile);
    Shader(Shader const&) = delete;
    Shader(Shader&&) = delete;
    
    ~Shader();
    
    int  get() const;
    
    int  load_source(char const* src);
    int  load_source(char const* path, ShaderFromFile);
    bool ready() const;
    std::string get_log() const;
private:
    unsigned _id;
};

class ShaderProgram {
public:
    ShaderProgram() = default;
    ShaderProgram(Shader const& vertex, Shader const& fragment);
    ShaderProgram(ShaderProgram const&) = delete;
    ShaderProgram(ShaderProgram&&) = delete;
    ~ShaderProgram();
    
    int  get() const;
    int  uniform(char const*) const;
    void use() const;
    static void unuse();
    
    int  attach(Shader const& vertex, Shader const& fragment);
    bool ready() const;
    std::string get_log() const;
private:
    unsigned _id {0u};
};

#endif // SHADER_HPP_INCLUDED
