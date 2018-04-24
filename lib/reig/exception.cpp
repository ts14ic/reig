#include "exception.h"

#include <sstream>

using namespace std::string_literals;

namespace reig::exception {
    FailedToLoadFontException::FailedToLoadFontException(std::string message)
            : message{move(message)} {}

    const char* FailedToLoadFontException::what() const noexcept {
        return message.c_str();
    }

    FailedToLoadFontException FailedToLoadFontException::no_texture_id(const char* file_path) {
        std::ostringstream ss;
        ss << "No texture id was specified for font: [" << file_path << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::invalid_height(const char* file_path, float font_size) {
        std::ostringstream ss;
        ss << "Invalid size specified for font: [" << file_path << "], size: [" << font_size << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::could_not_open_file(const char* file_path) {
        std::ostringstream ss;
        ss << "Could not open font file: [" << file_path << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::could_not_fit_characters(
            const char* file_path, float font_size, int width, int height) {
        std::ostringstream ss;
        ss << "Could not fit characters for font: ["
           << file_path << "], size: [" << font_size << "], atlas size: ["
           << width << "x" << height << "]";
        return FailedToLoadFontException(ss.str());
    }

    FailedToLoadFontException FailedToLoadFontException::invalid_file(const char* file_path) {
        std::ostringstream ss;
        ss << "Invalid file for font: [" << file_path << "]";
        return FailedToLoadFontException(ss.str());
    }

    const char* NoRenderHandlerException::what() const noexcept {
        return "No render handler specified";
    }

    IntegralCastException::IntegralCastException(long long val, const char* src_type, const char* dest_type)
            : std::range_error{"Bad integral cast from "s + src_type + " (" + std::to_string(val) + ") to " + dest_type} {
    }
}