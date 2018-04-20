#ifndef REIG_REFERENCE_WIDGET_H
#define REIG_REFERENCE_WIDGET_H

#include "fwd.h"
#include "text.h"
#include "primitive.h"

namespace reig::reference_widget {
    struct button {
        const char* const mTitle;
        const primitive::Rectangle mBoundingBox;
        const primitive::Color mBaseColor;

        /**
         * @brief Render a titled button
         *
         * @param title Text to be displayed on button
         * @param box Button's bounding box
         * @param color Button's base color
         *
         * @return True if the button was clicked, false otherwise
         */
        bool use(Context& ctx) const;
    };

    struct textured_button {
        const char* const mTitle;
        const primitive::Rectangle mBoundingBox;
        const int mHoverTexture = 0, mBaseTexture = 0;

        /**
         * @brief Render a titled textured button
         * @param box Button's bouding box
         * @param baseTexture Button's texture index, when idle
         * @param hoverTexture Button's texture index, when button is hoverred
         * @return True if the button was clicked, false otherwise
         */
        bool use(Context& ctx) const;
    };

    struct label {
        char const* mTitle = "";
        primitive::Rectangle mBoundingBox;
        text::Alignment mAlignment = text::Alignment::CENTER;
        float mFontScale = 1.f;

        /**
         * @brief Render a label, which will be enclosed in the current window, if any
         * @param text Text to be displayed
         * @param box Text's bounding box
         */
        void use(Context& ctx) const;
    };

    struct slider {
        primitive::Rectangle mBoundingBox;
        primitive::Color mBaseColor;
        float& mValueRef;
        float mMin = 0.0f, mMax = 0.0f, mStep = 0.0f;

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
        bool use(Context& ctx) const;
    };

    struct textured_slider {
        primitive::Rectangle mBoundingBox;
        int mBaseTexture = 0, mCursorTexture = 0;
        float& mValueRef;
        float mMin = 0.0f, mMax = 0.0f, mStep = 0.0f;

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
        bool use(Context& ctx) const;
    };

//    struct scrollbar {
//        primitive::Rectangle mBoundingBox;
//        primitive::Color mBaseColor;
//        float& mValueRef;
//        float mViewSize = 0.0f;
//
//        /**
//         * @brief Renders a vertical scrollbar
//         * @param box Scrollbar's position and size
//         * @param color Checkbox's base color
//         * @param value A reference to the float to be changed
//         * @return True if value changed
//         */
//        void use(Context& ctx, std::function<void()> callback = [](){}) const;
//    };
//
//    struct checkbox {
//        primitive::Rectangle mBoundingBox;
//        primitive::Color mBaseColor;
//        bool& mValueRef;
//        /**
//         * @brief Renders a checkbox
//         * @param box Checkbox's position and size
//         * @param color Checkbox's base color
//         * @param value A reference to the bool to be changed
//         * @return True if value changed
//         */
//        void use(Context& ctx, std::function<void()> callback = [](){}) const;
//    };
//
//    struct textured_checkbox {
//        primitive::Rectangle mBoundingBox;
//        int mBaseTexture = 0, mCheckTexture = 0;
//        bool& mValueRef;
//
//        /**
//         * @brief Renders a textured checkbox
//         * @param box Checkbox's position and size
//         * @param baseTexture Checkbox's base texture
//         * @param tickTexture Checkbox's tick texture
//         * @param value A reference to the bool to be changed
//         * @return True if value changed
//         */
//        void use(Context& ctx, std::function<void()> callback = [](){}) const;
//    };
}

#endif //REIG_REFERENCE_WIDGET_H
