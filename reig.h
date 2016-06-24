#ifndef REIG_H_INCLUDED
#define REIG_H_INCLUDED

namespace reig {
    inline namespace value_types {
        using ubyte_t = unsigned char;
        using int_t   = int;
        using uint_t  = unsigned;
        using size_t  = unsigned long;        
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
         */
        void click_mouse(bool clicked = true);
        
        // Widget renders
        /**
         * @brief Render a button
         * @param rect Button position and size
         * @param color Button's base color
         * @return True if the button was clicked, false otherwise
         */
        bool button(Rectangle rect, Color color);
    
    private:
        Point _mousePrevPos {};
        Point _mouseCurrPos {};
        bool  _mouseClicked = false;
    };
}

#endif // REIG_H_INCLUDED
