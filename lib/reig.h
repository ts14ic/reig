#ifndef REIG_H_INCLUDED
#define REIG_H_INCLUDED

#include "stb_truetype.h"
#include <vector>
#include <cstdint>
#include <any>
#include <string>

namespace reig {
    using ubyte_t = std::uint8_t;
    using int_t   = std::int32_t;
    using uint_t  = std::uint32_t;
    using size_t  = std::size_t;

    static_assert(sizeof(uint_t) >= 4 * sizeof(ubyte_t), "uint_t is too small");

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

    struct Red {
        ubyte_t val = 0u;

        constexpr explicit Red(ubyte_t val = 0u) noexcept : val{val} {}
    };

    struct Green {
        ubyte_t val = 0u;

        constexpr explicit Green(ubyte_t val = 0u) noexcept : val{val} {}
    };

    struct Blue {
        ubyte_t val = 0u;

        constexpr explicit Blue(ubyte_t val = 0u) noexcept : val{val} {}
    };

    struct Alpha {
        ubyte_t val = 0xFFu;

        constexpr explicit Alpha(ubyte_t val = 0xFFu) noexcept : val{val} {}
    };

    struct Color {
        constexpr Color() = default;

        constexpr Color(Red const& red, Green const& green, Blue const& blue,
                        Alpha const& alpha = Alpha{0xFFu}) noexcept
                : red{red}, green{green}, blue{blue}, alpha{alpha} {}

        Red red;
        Green green;
        Blue blue;
        Alpha alpha;
    };

    namespace Colors {
        auto to_uint(Color const& from) -> uint_t;

        auto from_uint(uint_t rgba) -> Color;

        namespace literals {
            constexpr auto operator ""_r(unsigned long long val) noexcept -> Red {
                return Red{static_cast<ubyte_t>(val)};
            }

            constexpr auto operator ""_g(unsigned long long val) noexcept -> Green {
                return Green{static_cast<ubyte_t>(val)};
            }

            constexpr auto operator ""_b(unsigned long long val) noexcept -> Blue {
                return Blue{static_cast<ubyte_t>(val)};
            }

            constexpr auto operator ""_a(unsigned long long val) noexcept -> Alpha {
                return Alpha{static_cast<ubyte_t>(val)};
            }
        }

        namespace mixing {
            namespace detail {
                template<typename Comp>
                static constexpr bool is_color_component_v = std::is_same_v<Comp, Red> ||
                                                             std::is_same_v<Comp, Green> ||
                                                             std::is_same_v<Comp, Blue> ||
                                                             std::is_same_v<Comp, Alpha>;

                template<typename Comp, typename = std::enable_if_t<is_color_component_v<Comp>>>
                auto get_comp(Color const& color) -> Comp const& {
                    if constexpr (std::is_same_v<Comp, Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Blue>) return color.blue;
                    else return color.alpha;
                }

                template<typename Comp, typename = std::enable_if_t<is_color_component_v<Comp>>>
                auto get_comp(Color& color) -> Comp& {
                    if constexpr (std::is_same_v<Comp, Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Blue>) return color.blue;
                    else return color.alpha;
                }
            }

            template<typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            auto operator|(Color const& left, Comp const& right) -> Color {
                Color ret = left;
                detail::get_comp<Comp>(ret) = right;
                return ret;
            };

            template<typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            auto operator+(Color const& left, Comp const& right) -> Color {
                Color ret = left;
                detail::get_comp<Comp>(ret).val += right.val;
                return ret;
            };

            template<typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            auto operator-(Color const& left, Comp const& right) -> Color {
                Color ret = left;
                detail::get_comp<Comp>(ret).val -= right.val;
                return ret;
            };
        }

        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        Color constexpr transparent{Red{}, Green{}, Blue{}, Alpha{}};
        Color constexpr red{Red{239}, Green{41}, Blue{41}};
        Color constexpr orange{Red{252}, Green{175}, Blue{62}};
        Color constexpr yellow{Red{252}, Green{233}, Blue{79}};
        Color constexpr green{Red{138}, Green{226}, Blue{52}};
        Color constexpr blue{Red{114}, Green{159}, Blue{207}};
        Color constexpr violet{Red{173}, Green{127}, Blue{168}};
        Color constexpr brown{Red{143}, Green{89}, Blue{2}};
        Color constexpr white{Red{255}, Green{255}, Blue{255}};
        Color constexpr lightGrey{Red{186}, Green{189}, Blue{182}};
        Color constexpr mediumGrey{Red{136}, Green{138}, Blue{133}};
        Color constexpr darkGrey{Red{46}, Green{52}, Blue{54}};
        Color constexpr black{Red{0}, Green{0}, Blue{0}};
        #pragma clang diagnostic pop
    }

    struct Vertex {
        Point position;
        Point texCoord;
        Color color;
    };

    /*forward*/ class Context;

    /*forward*/ class Mouse;

    class MouseButton {
    public:
        /**
         * @brief Sets mouse pressed and clicked states
         * @param x X coordinate
         * @param y Y coordinate
         */
        auto press(float x, float y) -> void;

        /**
         * @brief Unsets mouse pressed state
         */
        auto release() -> void;

        MouseButton() = default;

        MouseButton(MouseButton const&) = delete;

        MouseButton(MouseButton&&) = delete;

        MouseButton& operator=(MouseButton const&) = delete;

        MouseButton& operator=(MouseButton&&) = delete;

    private:
        friend class ::reig::Context;

        friend class ::reig::Mouse;

        Point mClickedPos;
        bool mIsPressed = false;
        bool mIsClicked = false;
    };

    class Mouse {
    public:
        Mouse() = default;

        Mouse(Mouse const&) = delete;

        Mouse(Mouse&&) = delete;

        Mouse& operator=(Mouse const&) = delete;

        Mouse& operator=(Mouse&&) = delete;

        MouseButton leftButton;
        MouseButton rightButton;

        /**
         * @brief Moves cursor against previous position
         * @param difx Delta x coordinate
         * @param dify Delta y coordinate
         */
        auto move(float difx, float dify) -> void;

        /**
         * @brief Places the cursors in abosulute coordinates
         * @param x X coordinate
         * @param y Y coordiante
         */
        auto place(float x, float y) -> void;

        /**
         * @brief Scrolls the virtual mouse wheel
         * @param dy Amount of scrolling
         */
        auto scroll(float dy) -> void;

    private:
        friend class ::reig::Context;

        Point mCursorPos;
        float mScrolled = 0.0f;
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
        auto vertices() const -> std::vector<Vertex> const&;

        /**
         * @brief Returns figure's read-only indices
         */
        auto indices() const -> std::vector<uint_t> const&;

        /**
         * @brief Return figure's texture index
         */
        auto texture() const -> uint_t;

    private:
        Figure() = default;

        friend class ::reig::Context;

        auto form(std::vector<Vertex>& vertices, std::vector<uint_t>& indices, uint_t id = 0) -> void;

        std::vector<Vertex> mVertices;
        std::vector<uint_t> mIndices;
        uint_t mTextureId = 0;
    };

    struct FontData {
        std::vector<ubyte_t> bitmap;
        uint_t width = 0;
        uint_t height = 0;
    };

    namespace detail {
        struct Font {
            std::vector<stbtt_bakedchar> bakedChars;
            float size = 0.f;
            uint_t texture = 0;
            uint_t width = 0;
            uint_t height = 0;
        };

        struct Window {
            std::vector<Figure> drawData;
            char const* title = nullptr;
            float* x = nullptr;
            float* y = nullptr;
            float w = 0.f;
            float h = 0.f;
            float headerSize = 0.f;
            bool started = false;

            auto expand(Rectangle& box) -> void;
        };
    }

    struct FailedToLoadFontException : std::exception {
    public:
        const char* what() const noexcept override;

        static FailedToLoadFontException noTextureId(const char* filePath);
        static FailedToLoadFontException invalidSize(const char* filePath, float fontSize);
        static FailedToLoadFontException couldNotOpenFile(const char* filePath);
        static FailedToLoadFontException couldNotFitCharacters(const char* filePath, float fontSize, uint_t width, uint_t height);
    private:
        explicit FailedToLoadFontException(std::string message);
        const std::string message;
    };

    struct NoRenderHandlerException : std::exception {
        const char* what() const noexcept override;
    };

    using DrawData = std::vector<Figure>;
    using RenderHandler = void (*)(DrawData const&, std::any&);

    /**
     * @class Context
     * @brief Used to pump in input and request gui creation
     */
    class Context {
    public:
        Context() = default;

        /**
         * @brief Set's a user function, which will draw the gui, based 
         * @param handler A C function pointer to a rendering callback
         * The handler should return void and take in DrawData const& and void*
         */
        auto set_render_handler(RenderHandler handler) -> void;

        /**
         * @brief Set a user pointer to store in the context.
         * @param ptr A user data pointer.
         * The pointer is then automatically passed to callbacks.
         */
        auto set_user_ptr(std::any ptr) -> void;

        /**
         * @brief Gets the stored user pointer
         */
        auto get_user_ptr() const -> std::any const&;

        /**
         * @brief Sets reig's font to be used for labels
         * @param fontFilePath The path to fonts .ttf file.
         * @param textureId This id will be passed by reig to render_handler with text vertices
         * @param fontHeightPx Font's pixel size
         * @return Returns the bitmap, which is used to create a texture by user.
         * Set returned bitmap field to nullptr, to avoid deletion
         */
        auto set_font(char const* fontFilePath, uint_t textureId, float fontHeightPx) -> FontData;

        /**
         * @brief Resets draw data and inputs
         */
        auto start_new_frame() -> void;

        /**
         * @brief Uses stored drawData and draws everything using the user handler
         */
        auto render_all() -> void;

        // Inputs
        Mouse mouse;

        // Widget renders
        auto start_window(char const* title, float& x, float& y) -> void;

        auto end_window() -> void;

        /**
         * @brief Render a label, which get's enclose in current window, if any
         * @param text Text to be displayed
         * @param box Text's bounding box
         */
        auto label(char const* text, Rectangle box) -> void;

        /**
         * @brief Render a titled button
         * @param title Text to be displayed on button
         * @param box Button's bounding box
         * @param color Button's base color
         * @return True if the button was clicked, false otherwise
         */
        auto button(char const* title, Rectangle box, Color color) -> bool;

        /**
         * @brief Render a titled textured button
         * @param box Button's bouding box
         * @param baseTexture Button's texture index, when idle
         * @param hoverTexture Button's texture index, when button is hoverred
         * @return True if the button was clicked, false otherwise
         */
        auto button(char const* title, Rectangle box, int baseTexture, int hoverTexture) -> bool;

        /**
         * @brief Renders a slider.
         * @param box Slider's bounding box
         * @param color Slider's base color
         * @param value A reference to the value to be represented and changed
         * @param min The lowest represantable value
         * @param max The highest represantable value
         * @param step The discrete portion by which the value can change
         * @return True if value changed
         */
        auto slider(Rectangle box, Color color, float& value, float min, float max, float step) -> bool;

        /**
         * @brief 
         * @brief Renders a slider.
         * @param box Slider's bounding box
         * @param baseTexture Slider's base texture index
         * @param cursorTexture Slider's cursor texture index
         * @param value A reference to the value to be represented and changed
         * @param min The lowest represantable value
         * @param max The highest represantable value
         * @param step The discrete portion by which the value can change
         * @return True if value changed
         */
        auto slider(Rectangle box, int baseTexture, int cursorTexture, float& value, float min, float max, float step) -> bool;

        /**
         * @brief Renders a checkbox
         * @param box Checkbox's position and size
         * @param color Checkbox's base color
         * @param value A reference to the bool to be changed
         * @return True if value changed
         */
        auto checkbox(Rectangle box, Color color, bool& value) -> bool;

        /**
         * @brief Renders a textured checkbox
         * @param box Checkbox's position and size
         * @param baseTexture Checkbox's base texture
         * @param tickTexture Checkbox's tick texture
         * @param value A reference to the bool to be changed
         * @return True if value changed
         */
        auto checkbox(Rectangle box, int baseTexture, int tickTexture, bool& value) -> bool;

        /**
         * @brief Renders a vertical scrollbar
         * @param box Scrollbar's position and size
         * @param color Checkbox's base color
         * @param value A reference to the bool to be changed
         * @param min Scrollbar's output min value
         * @param max Scrollbar's output max value
         * @param logicalHeight Max logical value 
         * @return 
         */
        //auto scrollbar(Rectangle box, Color color, float& value, float min, float max, float logicalHeight) -> bool;

        // Primitive renders
        /**
         * @brief Render some text
         * @param text Text to be displayed
         * @param box Text's bounding box
         */
        auto render_text(char const* text, Rectangle box) -> void;

        /**
         * @brief Schedules a rectangle drawing
         * @param rect Position and size
         * @param color Color
         */
        auto render_rectangle(Rectangle const& rect, Color const& color) -> void;

        /**
         * @brief Schedules a textured rectangle drawing (the texture is stretched)
         * @param rect Position and size
         * @param texture Index to the texture
         */
        auto render_rectangle(Rectangle const& rect, int texture) -> void;

        /**
         * @brief Schedules a triangle drawing
         * @param triangle Position and size
         * @param color Color
         */
        auto render_triangle(Triangle const& triangle, Color const& color) -> void;

    private:
        std::vector<Figure> mDrawData;
        detail::Font mFont;
        detail::Window mCurrentWindow;

        RenderHandler mRenderHandler = nullptr;
        std::any mUserPtr;
    };
}

#endif // REIG_H_INCLUDED
