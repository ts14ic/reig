#include "reig.h"

bool reig::helpers::in_box(Point const& pt, Rectangle const& box) {
    return between(pt.x, box.x, box.x + box.w) && between(pt.y, box.y, box.y + box.h);
}

void reig::Context::set_render_handler(void (*renderHandler)(DrawData const& drawData)) {
    _renderHandler = renderHandler;
}

void reig::Context::render_all() const {
    if(!_renderHandler) return;
    _renderHandler(_drawData);
}

void reig::Context::start_new_frame() {
    _inputs.mouseLeftClicked = false;
    _drawData.clear();
}

void reig::Context::move_mouse(float_t difx, float_t dify) {
    _inputs.mouseCurrPos.x += difx;
    _inputs.mouseCurrPos.y += dify;
}

void reig::Context::place_mouse(float_t x, float_t y) {
    _inputs.mouseCurrPos.x = x;
    _inputs.mouseCurrPos.y = y;
}

void reig::Context::click_mouse(Point const& pos) {
    _inputs.mouseClickedPos = pos;
    _inputs.mouseLeftClicked = true;
}

void reig::Context::Figure::form(std::vector<Vertex>& vertices, std::vector<uint_t>& indices) {
    vertices.swap(_vertices);
    indices.swap(_indices);
}

std::vector<reig::Vertex> const& reig::Context::Figure::vertices() const {
    return _vertices;
}

std::vector<reig::uint_t> const& reig::Context::Figure::indices() const {
    return _indices;
}

bool reig::Context::button(Rectangle aRect, Color aColor) {
    int darken = 0x50;
    Color outlineCol (
        (aColor.r > darken ? aColor.r - darken : aColor.r),
        (aColor.g > darken ? aColor.g - darken : aColor.g),
        (aColor.b > darken ? aColor.b - darken : aColor.b)
    );
    
    render_rectangle(aRect, outlineCol);
    
    aRect.x += 2; aRect.y += 2;
    aRect.w -= 4; aRect.h -= 4;
    if(in_box(_inputs.mouseCurrPos, aRect)) {
        aColor.r < 205 ? aColor.r += 50 : aColor.r = 255;
        aColor.g < 205 ? aColor.g += 50 : aColor.g = 255;
        aColor.b < 205 ? aColor.b += 50 : aColor.b = 255;
    }
    render_rectangle(aRect, aColor);
    return _inputs.mouseLeftClicked && in_box(_inputs.mouseClickedPos, aRect);
}

void reig::Context::render_triangle(Triangle const& aTri, Color const& aColor) {
    std::vector<Vertex> vertices (3);
    vertices[0].position = {aTri.pos0};
    vertices[1].position = {aTri.pos1};
    vertices[2].position = {aTri.pos2};
    
    for(auto& vert : vertices) {
        vert.color = aColor;
    }
    
    std::vector<uint_t> indices = {0, 1, 2};
    
    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}

void reig::Context::render_rectangle(Rectangle const& aRect, Color const& aColor) {
    std::vector<Vertex> vertices (4);
    vertices[0].position = Point{aRect.x          , aRect.y          };
    vertices[1].position = Point{aRect.x + aRect.w, aRect.y          };
    vertices[2].position = Point{aRect.x + aRect.w, aRect.y + aRect.h};
    vertices[3].position = Point{aRect.x          , aRect.y + aRect.h};
    
    for(auto& vert : vertices) {
        vert.color = aColor;
    }
    
    std::vector<uint_t> indices {0, 1, 2, 0, 2, 3};
    
    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}
