#ifndef REIG_CONFIG_H
#define REIG_CONFIG_H

#include "fwd.h"
#include "primitive.h"

namespace reig {
    struct ConfigBuilder;

    struct Config {
    public:
        static ConfigBuilder builder();

    private:
        friend Context;
        friend ConfigBuilder;

        explicit Config(const ConfigBuilder& builder);

        primitive::Color mWindowBackgroundColor;
        primitive::Color mTitleBackgroundColor;
        bool mWindowsTextured;
        int mWindowBackgroundTexture;
        int mTitleBackgroundTexture;
        int mFontBitmapWidth;
        int mFontBitmapHeight;
    };

    class ConfigBuilder {
    public:
        ConfigBuilder();

        ConfigBuilder& windowColors(const primitive::Color& titleBackground, const primitive::Color& windowBackground);

        ConfigBuilder& windowTextures(int titleTexture, int backgroundTexture);

        ConfigBuilder& fontBitmapSizes(int width, int height);

        Config build();
    private:
        friend Config;

        primitive::Color mWindowBackgroundColor = primitive::colors::transparent;
        primitive::Color mTitleBackgroundColor = primitive::colors::white;
        bool mWindowsTextured = false;
        int mWindowBackgroundTexture = 0;
        int mTitleBackgroundTexture = 0;
        int mFontBitmapWidth = 512;
        int mFontBitmapHeight = 512;
    };
}

#endif //REIG_CONFIG_H
