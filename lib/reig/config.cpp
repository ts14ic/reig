#include "config.h"

namespace reig {
    ConfigBuilder Config::builder() {
        return ConfigBuilder();
    }

    Config::Config(const ConfigBuilder& builder) {
        mWindowsTextured = builder.mWindowsTextured;
        if (mWindowsTextured) {
            mTitleBackgroundTexture = builder.mTitleBackgroundTexture;
            mWindowBackgroundTexture = builder.mWindowBackgroundTexture;
        } else {
            mTitleBackgroundColor = builder.mTitleBackgroundColor;
            mWindowBackgroundColor = builder.mWindowBackgroundColor;
        }
        mFontBitmapWidth = builder.mFontBitmapWidth;
        mFontBitmapHeight = builder.mFontBitmapHeight;
    }

    ConfigBuilder::ConfigBuilder() = default;

    ConfigBuilder& ConfigBuilder::windowColors(const primitive::Color& titleBackground,
                                               const primitive::Color& windowBackground) {
        mTitleBackgroundColor = titleBackground;
        mWindowBackgroundColor = windowBackground;
        mWindowsTextured = false;
        return *this;
    }

    ConfigBuilder& ConfigBuilder::windowTextures(int titleTexture, int backgroundTexture) {
        mTitleBackgroundTexture = titleTexture;
        mWindowBackgroundTexture = backgroundTexture;
        mWindowsTextured = true;
        return *this;
    }

    ConfigBuilder& ConfigBuilder::fontBitmapSizes(int width, int height) {
        mFontBitmapWidth = width;
        mFontBitmapHeight = height;
        return *this;
    }

    Config ConfigBuilder::build() {
        return Config(*this);
    }

}