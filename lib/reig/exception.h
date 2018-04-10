#ifndef REIG_EXCEPTION_H
#define REIG_EXCEPTION_H

#include <string>

namespace reig::exception {
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

#endif //REIG_EXCEPTION_H
