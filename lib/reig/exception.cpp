#include "exception.h"

#include <sstream>

using namespace std::string_literals;

namespace reig::exception {
    FailedToLoadFontException::FailedToLoadFontException(std::string message)
            : message{move(message)} {}

    const char* FailedToLoadFontException::what() const noexcept {
        return message.c_str();
    }

    FailedToLoadFontException FailedToLoadFontException::noTextureId(const char* filePath) {
        std::ostringstream ss;
        ss << "No texture id was specified for font: [" << filePath << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::invalidSize(const char* filePath, float fontSize) {
        std::ostringstream ss;
        ss << "Invalid size specified for font: [" << filePath << "], size: [" << fontSize << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::couldNotOpenFile(const char* filePath) {
        std::ostringstream ss;
        ss << "Could not open font file: [" << filePath << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::couldNotFitCharacters(
            const char* filePath, float fontSize, int width, int height) {
        std::ostringstream ss;
        ss << "Could not fit characters for font: ["
           << filePath << "], size: [" << fontSize << "], atlas size: ["
           << width << "x" << height << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::invalidFile(const char* filePath) {
        std::ostringstream ss;
        ss << "Invalid file for font: [" << filePath << "]";
        return FailedToLoadFontException(ss.str());
    }

    const char* NoRenderHandlerException::what() const noexcept {
        return "No render handler specified";
    }

    IntegralCastException::IntegralCastException(long long val, const char* srcType, const char* destType)
            : std::range_error{"Bad integral cast from "s + srcType + " (" + std::to_string(val) + ") to " + destType} {
    }
}