#ifndef REIG_H_INCLUDED
#define REIG_H_INCLUDED

#include "stb_truetype.h"
#include <vector>
#include <cstdint>
#include <any>
#include <string>

namespace reig {
    using uint8_t = std::uint8_t;
    using uint32_t = std::uint32_t;

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

    namespace Colors {
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
                template<typename Comp>
                static constexpr bool is_color_component_v = std::is_same_v<Comp, Color::Red> ||
                                                             std::is_same_v<Comp, Color::Green> ||
                                                             std::is_same_v<Comp, Color::Blue> ||
                                                             std::is_same_v<Comp, Color::Alpha>;

                template<typename Comp, typename = std::enable_if_t<is_color_component_v<Comp>>>
                Comp const& get_comp(Color const& color) {
                    if constexpr (std::is_same_v<Comp, Color::Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Color::Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Color::Blue>) return color.blue;
                    else return color.alpha;
                }

                template<typename Comp, typename = std::enable_if_t<is_color_component_v<Comp>>>
                Comp& get_comp(Color& color) {
                    if constexpr (std::is_same_v<Comp, Color::Red>) return color.red;
                    else if constexpr (std::is_same_v<Comp, Color::Green>) return color.green;
                    else if constexpr (std::is_same_v<Comp, Color::Blue>) return color.blue;
                    else return color.alpha;
                }
            }

            template<typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator|(Color const& left, Comp const& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret) = right;
                return ret;
            };

            template<typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator+(Color const& left, Comp const& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret).val += right.val;
                return ret;
            };

            template<typename Comp, typename = std::enable_if_t<detail::is_color_component_v<Comp>>>
            Color operator-(Color const& left, Comp const& right) {
                Color ret = left;
                detail::get_comp<Comp>(ret).val -= right.val;
                return ret;
            };
        }

        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        // @formatter:off
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
        // @formatter:on
        #pragma clang diagnostic pop
    }

    struct Vertex {
        Point position;
        Point texCoord;
        Color color;
    };

    /* forward */ class Context;

    namespace detail {
        class MouseButton {
        public:
            MouseButton() = default;

            MouseButton(MouseButton const&) = delete;

            MouseButton(MouseButton&&) = delete;

            MouseButton& operator=(MouseButton const&) = delete;

            MouseButton& operator=(MouseButton&&) = delete;

            /**
             * @brief Sets mouse pressed and clicked states
             * @param x X coordinate
             * @param y Y coordinate
             */
            void press(float x, float y);

            /**
             * @brief Unsets mouse pressed state
             */
            void release();

            const Point& get_clicked_pos() const;

            bool is_pressed() const;

            bool is_clicked() const;

        private:
            friend class ::reig::Context;

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

            detail::MouseButton leftButton;
            detail::MouseButton rightButton;

            /**
             * @brief Moves cursor against previous position
             * @param difx Delta x coordinate
             * @param dify Delta y coordinate
             */
            void move(float difx, float dify);

            /**
             * @brief Places the cursors in abosulute coordinates
             * @param x X coordinate
             * @param y Y coordiante
             */
            void place(float x, float y);

            /**
             * @brief Scrolls the virtual mouse wheel
             * @param dy Amount of scrolling
             */
            void scroll(float dy);

            const Point& get_cursor_pos() const;

            float get_scrolled() const;

        private:
            friend class ::reig::Context;

            Point mCursorPos;
            float mScrolled = 0.0f;
        };
    }

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

    struct FontData {
        std::vector<uint8_t> bitmap;
        int width = 0;
        int height = 0;
    };

    namespace detail {
        struct Font {
            std::vector<stbtt_bakedchar> bakedChars;
            float size = 0.f;
            int texture = 0;
            int width = 0;
            int height = 0;
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

            void expand(Rectangle& box);
        };
    }

    namespace except {
        struct FailedToLoadFontException : std::exception {
        public:
            const char* what() const noexcept override;

            static FailedToLoadFontException noTextureId(const char* filePath);
            static FailedToLoadFontException invalidSize(const char* filePath, float fontSize);
            static FailedToLoadFontException couldNotOpenFile(const char* filePath);
            static FailedToLoadFontException invalidFile(const char* filePath);
            static FailedToLoadFontException couldNotFitCharacters(const char* filePath, float fontSize, int width, int height);
        private:
            explicit FailedToLoadFontException(std::string message);
            const std::string message;
        };

        struct NoRenderHandlerException : std::exception {
            const char* what() const noexcept override;
        };
    }

    namespace reference_widgets {
        struct button {
            char const* mTitle;
            Rectangle mBoundingBox;
            Color mBaseColor;

            /**
             * @brief Render a titled button
             *
             * @param title Text to be displayed on button
             * @param box Button's bounding box
             * @param color Button's base color
             *
             * @return True if the button was clicked, false otherwise
             */
            bool draw(Context& ctx) const;
        };

        struct textured_button {
            char const* mTitle;
            Rectangle mBoundingBox;
            int mBaseTexture, mHoverTexture;

            /**
             * @brief Render a titled textured button
             * @param box Button's bouding box
             * @param baseTexture Button's texture index, when idle
             * @param hoverTexture Button's texture index, when button is hoverred
             * @return True if the button was clicked, false otherwise
             */
            bool draw(Context& ctx) const;
        };

        struct label {
            char const* mTitle;
            Rectangle mBoundingBox;
            /**
             * @brief Render a label, which will be enclosed in the current window, if any
             * @param text Text to be displayed
             * @param box Text's bounding box
             */
            void draw(Context& ctx) const;
        };

        struct slider {
            Rectangle mBoundingBox;
            Color mBaseColor;
            float& mValueRef;
            float mMin, mMax, mStep;

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
            bool draw(Context& ctx) const;
        };

        struct slider_textured {
            Rectangle mBoundingBox;
            int mBaseTexture, mCursorTexture;
            float& mValueRef;
            float mMin, mMax, mStep;

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
            bool draw(Context& ctx) const;
        };

        struct checkbox {
            Rectangle mBoundingBox;
            Color mBaseColor;
            bool& mValueRef;
            /**
             * @brief Renders a checkbox
             * @param box Checkbox's position and size
             * @param color Checkbox's base color
             * @param value A reference to the bool to be changed
             * @return True if value changed
             */
            bool draw(Context& ctx) const;
        };

        struct textured_checkbox {
            Rectangle mBoundingBox;
            int mBaseTexture, mCheckTexture;
            bool& mValueRef;

            /**
             * @brief Renders a textured checkbox
             * @param box Checkbox's position and size
             * @param baseTexture Checkbox's base texture
             * @param tickTexture Checkbox's tick texture
             * @param value A reference to the bool to be changed
             * @return True if value changed
             */
            bool draw(Context& ctx) const;
        };
    }

    /**
     * @class Context
     * @brief Used to pump in input and request gui creation
     */
    class Context {
    public:
        Context() = default;

        using DrawData = std::vector<Figure>;
        using RenderHandler = void (*)(DrawData const&, std::any&);

        /**
         * @brief Set's a user function, which will draw the gui, based 
         * @param handler A C function pointer to a rendering callback
         * The handler should return void and take in DrawData const& and void*
         */
        void set_render_handler(RenderHandler handler);

        /**
         * @brief Set a user pointer to store in the context.
         * @param ptr A user data pointer.
         * The pointer is then automatically passed to callbacks.
         */
        void set_user_ptr(std::any ptr);

        /**
         * @brief Gets the stored user pointer
         */
        std::any const& get_user_ptr() const;

        /**
         * @brief Sets reig's font to be used for labels
         * @param fontFilePath The path to fonts .ttf file.
         * @param textureId This id will be passed by reig to render_handler with text vertices
         * @param fontHeightPx Font's pixel size
         * @return Returns the bitmap, which is used to create a texture by user.
         * Set returned bitmap field to nullptr, to avoid deletion
         */
        FontData set_font(char const* fontFilePath, int textureId, float fontHeightPx);

        /**
         * @brief Resets draw data and inputs
         */
        void start_new_frame();

        /**
         * @brief Uses stored drawData and draws everything using the user handler
         */
        void render_all();

        // Inputs
        detail::Mouse mouse;

        // Widget renders
        void start_window(char const* title, float& x, float& y);

        void end_window();

        template <typename T>
        auto widget(T&& t) {
            return t.draw(*this);
        }

        void fit_rect_in_window(Rectangle& rect);

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
        //bool scrollbar(Rectangle box, Color color, float& value, float min, float max, float logicalHeight);

        // Primitive renders
        /**
         * @brief Render some text
         * @param text Text to be displayed
         * @param box Text's bounding box
         */
        void render_text(char const* text, Rectangle box);

        /**
         * @brief Schedules a rectangle drawing
         * @param rect Position and size
         * @param color Color
         */
        void render_rectangle(Rectangle const& rect, Color const& color);

        /**
         * @brief Schedules a textured rectangle drawing (the texture is stretched)
         * @param rect Position and size
         * @param texture Index to the texture
         */
        void render_rectangle(Rectangle const& rect, int texture);

        /**
         * @brief Schedules a triangle drawing
         * @param triangle Position and size
         * @param color Color
         */
        void render_triangle(Triangle const& triangle, Color const& color);

    private:
        std::vector<Figure> mDrawData;
        detail::Font mFont;
        detail::Window mCurrentWindow;

        RenderHandler mRenderHandler = nullptr;
        std::any mUserPtr;
    };
}

#endif // REIG_H_INCLUDED
