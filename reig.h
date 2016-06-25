#ifndef REIG_H_INCLUDED
#define REIG_H_INCLUDED

#include <vector>

namespace reig {
    inline namespace value_types {
        using ubyte_t = unsigned char;
        using int_t   = signed;
        using uint_t  = unsigned;
        using size_t  = unsigned long long;        
        using float_t = float;
        
        static_assert(sizeof(uint_t) >= 4 * sizeof(ubyte_t), "uint_t is too small");
    }
    
    inline namespace graphic_base_types {
        struct Point {
            Point(float_t x = {}, float_t y = {})
                : x{x}, y{y} {}
            
            float_t x;
            float_t y;
        };
        
        struct Size {
            Size(float_t w = {}, float_t h = {})
                : w{w}, h{h} {}
            
            float_t w;
            float_t h;
        };
        
        struct Rectangle {
            Rectangle() = default;
            Rectangle(float_t x, float_t y, float_t w, float_t h)
                : x{x}, y{y}, w{w}, h{h} {}
            Rectangle(Point pos, Size size) 
                : pos{pos}, size{size} {}
            
            union {
                Point pos {};
                struct { float_t x; float_t y; };
            };
            union {
                Size size {};
                struct { float_t w; float_t h; };
            };
        };
        
        struct Triangle {
            Triangle() = default;
            Triangle(float_t x0, float_t y0, float_t x1, float_t y1, float_t x2, float_t y2)
                : pos0{x0, y0}, pos1{x1, y1}, pos2{x2, y2} {}
            Triangle(Point const& pos0, Point const& pos1, Point const& pos2)
                : pos0{pos0}, pos1{pos1}, pos2{pos2} {}
                
            union {
                Point pos0 {};
                struct { float_t x0; float_t y0; };
            };
            union {
                Point pos1 {};
                struct { float_t x1; float_t y1; };
            };
            union {
                Point pos2 {};
                struct { float_t x2; float_t y2; };
            };
        };
        
        struct Color {
            Color(ubyte_t r = {}, ubyte_t g = {}, ubyte_t b = {}, ubyte_t a = {0xFFu})
                : r{r}, g{g}, b{b}, a{a} {}
            
            union {
                struct {
                    ubyte_t r;
                    ubyte_t g;
                    ubyte_t b;
                    ubyte_t a;
                };
                uint_t asUint;
            };
        };
        
        using Colour = Color;
        
        struct Vertex {
            Vertex() = default;
            
            Point position {};
            Color color {};
        };
    }
    
    inline namespace helpers {
        /**
         * @brief Find if a value is between two other
         * @param val Value to check
         * @param min Lower boundary
         * @param max Upper boundary
         * @return true, if val is bigger than min and smaller than max
         */
        template <typename T>
        bool between(T val, T min, T max) {
            return val > min && val < max;
        }
        
        /**
         * @brief Find if a point is within a rectangle
         * @param pt Point to be checked
         * @param box Rectangle to be checked
         * @return true if pt is within box
         */
        bool in_box(Point const& pt, Rectangle const& box);
    }
    
    /**
     * @class Context
     * @brief Used to pump in input and request gui creation
     */
    class Context {
    public:
        Context() = default;
        
        class Figure;
        using DrawData = std::vector<Figure>;
        
        /**
         * @brief Set's a user function, which will draw the gui, based 
         * @param drawData
         */
        void set_render_handler(void (*handler)(DrawData const& drawData));
        
        /**
         * @brief Uses stored drawData and draws everything using the user handler
         */
        void render_all() const;
        
        /**
         * @brief Resets draw data and inputs
         */
        void start_new_frame();
        
        // Context inputs
        /**
         * @brief Moves cursor against previous position
         * @param difx Delta x coordinate
         * @param dify Delta y coordinate
         */
        void move_mouse(float_t difx, float_t dify);
        /**
         * @brief Places the cursors in abosulute coordinates
         * @param x X coordinate
         * @param y Y coordiante
         */
        void place_mouse(float_t x, float_t y);
        
        /**
         * @brief Sets mouse clicked state
         * @param position X and Y of where the mouse was clicked
         */
        void click_mouse(Point const& position);
        
        // Widget renders
        /**
         * @brief Render a button
         * @param box Button's bounding box
         * @param color Button's base color
         * @return True if the button was clicked, false otherwise
         */
        bool button(Rectangle box, Color color);
        
        /**
         * @brief Renders a slider. Integer overload.
         * @param box Slider's bounding box
         * @param color Slider's base color
         * @param value The value to be represented on slider
         * @param min The lowest represantable value
         * @param max The highest represantable value
         * @param step The discrete portion by which the value can change
         * @return The new value is returned, so the slider can be used 
         */
        int_t slider(Rectangle box, Color color, int_t value, int_t min, int_t max, int_t step);
        
        /**
         * @brief Renders a slider. Float overload.
         * @param box Slider's bounding box
         * @param color Slider's base color
         * @param value The value to be represented on slider
         * @param min The lowest represantable value
         * @param max The highest represantable value
         * @param step The discrete portion by which the value can change
         * @return The new value is returned, so the slider can be used 
         */
        float_t slider(Rectangle box, Color color, float_t value, float_t min, float_t max, float_t step);
         
        // Render primitives
        /**
         * @brief Schedules a rectangle drawing
         * @param rect Position and size
         * @param color Color
         */
        void render_rectangle(Rectangle const& rect, Color const& color);
        
        /**
         * @brief Schedules a triangle drawing
         * @param triangle Position and size
         * @param color Color
         */
        void render_triangle(Triangle const& triangle, Color const& color);
        
        /**
         * @class Figure
         * @brief A bunch of vertices and indices to render a figure
         * Can be collected by the user, but formation is accessible only for the Context
         */
        class Figure {
        public:
            Figure() = default;
            
            /**
             * @brief Returns figure's read-only vertices
             */
            std::vector<Vertex> const& vertices() const;
            /**
             * @brief Returns figure's read-only indices
             */
            std::vector<uint_t> const&  indices() const;
        private:
            friend class Context;
            
            void form(std::vector<Vertex>& vertices, std::vector<uint_t>& indices);
        
            std::vector<Vertex> _vertices;
            std::vector<uint_t>  _indices;
        };
    
    private:
        struct {
            Point mouseCurrPos;
            Point mouseClickedPos;
            bool  mouseLeftClicked = false;
        } _inputs;
        
        std::vector<Figure> _drawData;
        
        void (*_renderHandler)(DrawData const&);
    };
}

#endif // REIG_H_INCLUDED
