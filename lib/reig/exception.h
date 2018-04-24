#ifndef REIG_EXCEPTION_H
#define REIG_EXCEPTION_H

#include <string>
#include <stdexcept>

namespace reig::exception {
    struct FailedToLoadFontException : std::exception {
    public:
        const char* what() const noexcept override;

        static FailedToLoadFontException no_texture_id(const char* filePath);
        static FailedToLoadFontException invalid_height(const char* filePath, float fontSize);
        static FailedToLoadFontException could_not_open_file(const char* filePath);
        static FailedToLoadFontException invalid_file(const char* filePath);
        static FailedToLoadFontException could_not_fit_characters(const char* filePath, float fontSize, int width,
                                                                  int height);
    private:
        explicit FailedToLoadFontException(std::string message);
        const std::string message;
    };

    struct NoRenderHandlerException : std::exception {
        const char* what() const noexcept override;
    };

    struct IntegralCastException : std::range_error {
        IntegralCastException(long long val, const char* srcType, const char* destType);
    };
}

#endif //REIG_EXCEPTION_H
