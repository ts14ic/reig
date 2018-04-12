#ifndef REIG_PRIMITIVE_H
#define REIG_PRIMITIVE_H

#include "defs.h"
#include "fwd.h"
#include <vector>

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

        constexpr Color() = default;

        constexpr Color(Red const& red, Green const& green, Blue const& blue,
                        Alpha const& alpha = Alpha{0xFFu}) noexcept
                : red{red}, green{green}, blue{blue}, alpha{alpha} {}

        Red red;
        Green green;
        Blue blue;
        Alpha alpha;
    };

    namespace colors {
        uint32_t to_uint(Color const& from);

        Color from_uint(uint32_t rgba);

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
                Comp const& get_comp(Color const& color) {
                    if constexpr (std::is_same_v<Comp, Color::Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Color::Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Color::Blue>) return color.blue;
                    else return color.alpha;
                }

                template <typename Comp, typename = std::enable_if_t<is_color_component_v<Comp>>>
                Comp& get_comp(Color& color) {
                    if constexpr (std::is_same_v<Comp, Color::Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Color::Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Color::Blue>) return color.blue;
                    else return color.alpha;
                }
            }

            template <typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator|(Color const& left, Comp const& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret) = right;
                return ret;
            };

            template <typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator+(Color const& left, Comp const& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret).val += right.val;
                return ret;
            };

            template <typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator-(Color const& left, Comp const& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret).val -= right.val;
                return ret;
            };
        }

        // @formatter:off
            #pragma clang diagnostic push
            #pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
            Color constexpr transparent{Color::Red{},    Color::Green{},    Color::Blue{},    Color::Alpha{}};
            Color constexpr red        {Color::Red{239}, Color::Green{41},  Color::Blue{41}};
            Color constexpr orange     {Color::Red{252}, Color::Green{175}, Color::Blue{62}};
            Color constexpr yellow     {Color::Red{252}, Color::Green{233}, Color::Blue{79}};
            Color constexpr green      {Color::Red{138}, Color::Green{226}, Color::Blue{52}};
            Color constexpr blue       {Color::Red{114}, Color::Green{159}, Color::Blue{207}};
            Color constexpr violet     {Color::Red{173}, Color::Green{127}, Color::Blue{168}};
            Color constexpr brown      {Color::Red{143}, Color::Green{89},  Color::Blue{2}};
            Color constexpr white      {Color::Red{255}, Color::Green{255}, Color::Blue{255}};
            Color constexpr lightGrey  {Color::Red{186}, Color::Green{189}, Color::Blue{182}};
            Color constexpr mediumGrey {Color::Red{136}, Color::Green{138}, Color::Blue{133}};
            Color constexpr darkGrey   {Color::Red{46},  Color::Green{52},  Color::Blue{54}};
            Color constexpr black      {Color::Red{0},   Color::Green{0},   Color::Blue{0}};
            #pragma clang diagnostic pop
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
        std::vector<Vertex> const& vertices() const;

        /**
         * @brief Returns figure's read-only indices
         */
        std::vector<int> const& indices() const;

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

#endif //REIG_PRIMITIVE_H