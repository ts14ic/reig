#ifndef REIG_DRAWDATA_H
#define REIG_DRAWDATA_H

#include "primitive.h"
#include <any>

namespace reig {
    using DrawData = std::vector<primitive::Figure>;
    using RenderHandler = void(*)(const DrawData&, std::any userPtr);
}

#endif //REIG_DRAWDATA_H
