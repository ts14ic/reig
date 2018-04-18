#include "primitive.h"

namespace reig::primitive {
    using std::vector;

    uint32_t colors::to_uint(Color const& color) {
        return (color.alpha.val << 24)
               + (color.blue.val << 16)
               + (color.green.val << 8)
               + color.red.val;
    }

    Color colors::from_uint(uint32_t rgba) {
        return Color{
                Color::Red{static_cast<uint8_t>((rgba >> 24) & 0xFF)},
                Color::Green{static_cast<uint8_t>((rgba >> 16) & 0xFF)},
                Color::Blue{static_cast<uint8_t>((rgba >> 8) & 0xFF)},
                Color::Alpha{static_cast<uint8_t>(rgba & 0xFF)}
        };
    }

    void Figure::form(vector<Vertex>& vertices, vector<int>& indices, int id) {
        vertices.swap(mVertices);
        indices.swap(mIndices);
        mTextureId = id;
    }

    vector<Vertex> const& Figure::vertices() const {
        return mVertices;
    }

    vector<int> const& Figure::indices() const {
        return mIndices;
    }

    int Figure::texture() const {
        return mTextureId;
    }
}
