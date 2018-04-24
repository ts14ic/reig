#ifndef REIG_EXCEPTION_H
#define REIG_EXCEPTION_H

#include <string>
#include <stdexcept>

namespace reig::exception {
    struct FailedToLoadFontException : std::exception {
    public:
        const char* what() const noexcept override;

        static FailedToLoadFontException no_texture_id(const char* file_path);
        static FailedToLoadFontException invalid_height(const char* file_path, float font_size);
        static FailedToLoadFontException could_not_open_file(const char* file_path);
        static FailedToLoadFontException invalid_file(const char* file_path);
        static FailedToLoadFontException could_not_fit_characters(const char* file_path, float font_size,
                                                                  int width, int height);
    private:
        explicit FailedToLoadFontException(std::string message);
        const std::string message;
    };

    struct NoRenderHandlerException : std::exception {
        const char* what() const noexcept override;
    };

    struct IntegralCastException : std::range_error {
        IntegralCastException(long long val, const char* src_type, const char* dest_type);
    };
}

#endif //REIG_EXCEPTION_H
