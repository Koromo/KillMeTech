#ifndef _KILLME_MATERIALDRAWER_H_
#define _KILLME_MATERIALDRAWER_H_

#include <memory>

namespace killme
{
    class VertexData;
    class Material;

    class MaterialDrawer
    {
    private:
        std::shared_ptr<Material> material_;
        std::shared_ptr<VertexData> vertexData_;

    public:
        /** Constructs */

    };
}

#endif