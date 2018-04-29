#ifndef REIG_EXCEPTION_H
#define REIG_EXCEPTION_H

#include "gsl.h"
#include <string>
#include <stdexcept>

namespace reig::exception {
    struct FailedToLoadFontException : std::exception {
    public:
        gsl::czstring what() const noexcept override;

        static FailedToLoadFontException no_texture_id(gsl::czstring file_path);
        static FailedToLoadFontException invalid_height(gsl::czstring file_path, float font_size);
        static FailedToLoadFontException could_not_open_file(gsl::czstring file_path);
        static FailedToLoadFontException invalid_file(gsl::czstring file_path);
        static FailedToLoadFontException could_not_fit_characters(gsl::czstring file_path, float font_size,
                                                                  int width, int height);
    private:
        explicit FailedToLoadFontException(std::string message);
        const std::string message;
    };

    struct NoRenderHandlerException : std::exception {
        gsl::czstring what() const noexcept override;
    };

    struct IntegralCastException : std::range_error {
        IntegralCastException(long long val, gsl::czstring src_type, gsl::czstring dest_type);
    };
}

#endif //REIG_EXCEPTION_H
