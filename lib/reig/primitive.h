#ifndef REIG_PRIMITIVE_H
#define REIG_PRIMITIVE_H

#include "defs.h"
#include "fwd.h"
#include <vector>
#include <array>

namespace reig::primitive {
    struct Point {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Rectangle {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    inline float get_x2(const Rectangle& rect) {
        return rect.x + rect.width;
    }

    inline float get_y2(const Rectangle& rect) {
        return rect.y + rect.height;
    }

    bool is_point_in_rect(const Point& pt, const Rectangle& rect);

    Rectangle decrease_rect(Rectangle aRect, int by);

    std::array<Rectangle, 4> get_rect_frame(const Rectangle& rect, float thickness);

    void trim_rect_in_other(Rectangle& fitted, const Rectangle& container);

    struct Triangle {
        Point pos0;
        Point pos1;
        Point pos2;
    };

    struct Color {
        struct Red {
            uint8_t val = 0u;

            constexpr explicit Red(uint8_t val = 0u) noexcept
                    : val{val} {}
        };

        struct Green {
            uint8_t val = 0u;

            constexpr explicit Green(uint8_t val = 0u) noexcept
                    : val{val} {}
        };

        struct Blue {
            uint8_t val = 0u;

            constexpr explicit Blue(uint8_t val = 0u) noexcept
                    : val{val} {}
        };

        struct Alpha {
            uint8_t val = 0xFFu;

            constexpr explicit Alpha(uint8_t val = 0xFFu) noexcept
                    : val{val} {}
        };

        constexpr Color() noexcept
                : Color{0, 0, 0, 0xFFu} {}

        constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 0xFFu) noexcept
                : red{red}, green{green}, blue{blue}, alpha{alpha} {}

        constexpr Color(const Red& red, const Green& green, const Blue& blue,
                        const Alpha& alpha = Alpha{0xFFu}) noexcept
                : Color{red.val, green.val, blue.val, alpha.val} {}

        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };

    namespace colors {
        uint32_t to_uint(const Color& from);

        Color from_uint(uint32_t rgba);

        Color get_yiq_contrast(Color color);

        Color lighten_color_by(Color color, uint8_t delta);

        Color dim_color_by(Color color, uint8_t delta);

        namespace literals {
            constexpr Color::Red operator "" _r(unsigned long long val) noexcept {
                return Color::Red{static_cast<uint8_t>(val)};
            }

            constexpr Color::Green operator "" _g(unsigned long long val) noexcept {
                return Color::Green{static_cast<uint8_t>(val)};
            }

            constexpr Color::Blue operator "" _b(unsigned long long val) noexcept {
                return Color::Blue{static_cast<uint8_t>(val)};
            }

            constexpr Color::Alpha operator "" _a(unsigned long long val) noexcept {
                return Color::Alpha{static_cast<uint8_t>(val)};
            }
        }

        namespace operators {
            namespace detail {
                template <typename Comp>
                static constexpr bool is_color_component_v = std::is_same_v<Comp, Color::Red> ||
                                                             std::is_same_v<Comp, Color::Green> ||
                                                             std::is_same_v<Comp, Color::Blue> ||
                                                             std::is_same_v<Comp, Color::Alpha>;

                template <typename Comp, typename = std::enable_if_t<is_color_component_v<Comp>>>
                uint8_t& get_comp(Color& color) {
                    if constexpr (std::is_same_v<Comp, Color::Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Color::Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Color::Blue>) return color.blue;
                    else return color.alpha;
                }
            }

            template <typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator|(const Color& left, const Comp& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret) = right.val;
                return ret;
            };

            template <typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator+(const Color& left, const Comp& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret) += right.val;
                return ret;
            };

            template <typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator-(const Color& left, const Comp& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret) -= right.val;
                return ret;
            };
        }

        // @formatter:off
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-variable"
        #pragma GCC diagnostic ignored "-Wunknown-pragmas"
        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        Color constexpr transparent{0, 0, 0, 0};
        Color constexpr red        {239, 41,  41};
        Color constexpr orange     {252, 175, 62};
        Color constexpr yellow     {252, 233, 79};
        Color constexpr green      {138, 226, 52};
        Color constexpr blue       {114, 159, 207};
        Color constexpr violet     {173, 127, 168};
        Color constexpr brown      {143, 89,  2};
        Color constexpr white      {255, 255, 255};
        Color constexpr lightGrey  {186, 189, 182};
        Color constexpr mediumGrey {136, 138, 133};
        Color constexpr darkGrey   {46,  52,  54};
        Color constexpr black      {0,   0,   0};
        #pragma clang diagnostic pop
        #pragma GCC diagnostic pop
        // @formatter:on
    }

    struct Vertex {
        Point position;
        Point texCoord;
        Color color;
    };

    /**
     * @class Figure
     * @brief A bunch of vertices and indices to render a figure
     * Can be collected by the user, but formation is accessible only for the Context
     */
    class Figure {
    public:
        /**
         * @brief Returns figure's read-only vertices
         */
        const std::vector<Vertex>& vertices() const;

        /**
         * @brief Returns figure's read-only indices
         */
        const std::vector<int>& indices() const;

        /**
         * @brief Return figure's texture index
         */
        int texture() const;

    private:
        Figure() = default;

        friend class ::reig::Context;

        void form(std::vector<Vertex>& vertices, std::vector<int>& indices, int id = 0);

        std::vector<Vertex> mVertices;
        std::vector<int> mIndices;
        int mTextureId = 0;
    };
}

namespace reig {
    using DrawData = std::vector<primitive::Figure>;
}

#endif //REIG_PRIMITIVE_H
