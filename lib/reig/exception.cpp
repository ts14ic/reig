#include "exception.h"

#include <sstream>

reig::exception::FailedToLoadFontException::FailedToLoadFontException(std::string message)
        : message{std::move(message)} {}

const char* reig::exception::FailedToLoadFontException::what() const noexcept {
    return message.c_str();
}

reig::exception::FailedToLoadFontException reig::exception::FailedToLoadFontException::noTextureId(const char* filePath) {
    std::ostringstream ss;
    ss << "No texture id was specified for font: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

reig::exception::FailedToLoadFontException reig::exception::FailedToLoadFontException::invalidSize(const char* filePath, float fontSize) {
    std::ostringstream ss;
    ss << "Invalid size specified for font: [" << filePath << "], size: [" << fontSize << "]";
    return FailedToLoadFontException(ss.str());
}

reig::exception::FailedToLoadFontException reig::exception::FailedToLoadFontException::couldNotOpenFile(const char* filePath) {
    std::ostringstream ss;
    ss << "Could not open font file: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

reig::exception::FailedToLoadFontException reig::exception::FailedToLoadFontException::couldNotFitCharacters(
        const char* filePath, float fontSize, int width, int height) {
    std::ostringstream ss;
    ss << "Could not fit characters for font: ["
       << filePath << "], size: [" << fontSize << "], atlas size: ["
       << width << "x" << height << "]";
    return FailedToLoadFontException(ss.str());
}

reig::exception::FailedToLoadFontException reig::exception::FailedToLoadFontException::invalidFile(const char* filePath) {
    std::ostringstream ss;
    ss << "Invalid file for font: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}
