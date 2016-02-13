#include "material.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../core/exception.h"
#include "../core/string.h"
#include "../resource/resourcemanager.h"
#include <fstream>
#include <array>

namespace killme
{
    namespace
    {
        std::array<std::string, 2> parse(const std::string& str)
        {
            const auto p = str.find(':');
            return{ toLowers(str.substr(0, p)), toLowers(str.substr(p + 1)) };
        }
    }

    std::shared_ptr<Material> loadMaterial(const std::string& path)
    {
        std::ifstream file(path);
        enforce<FileException>(file.is_open(), "Failed to open file (" + path + ").");

        Resource<VertexShader> vs;
        Resource<PixelShader> ps;

        for (int i = 0; i < 2; ++i)
        {
            std::string line;
            std::getline(file, line);

            const auto parsed = parse(line);
            const auto shaderPath = parsed[1];

            if (parsed[0] == "vertexshader")
            {
                vs = getManagedResource<VertexShader>(shaderPath);
            }
            else
            {
                ps = getManagedResource<PixelShader>(shaderPath);
            }
        }

        return std::make_shared<Material>(vs, ps);
    }
}