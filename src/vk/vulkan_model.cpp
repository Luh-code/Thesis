#include "vulkan_base.h"
#include "../pch.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Ths::Vk
{
  bool loadModel(VContext* pContext)
  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // std::string basedir = "assets/models/";
    std::string basedir = BASE_PATH + "assets/models";
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), basedir.c_str(), true, true))
    {
      LOG_ERROR("An error occured whilst loading \"", MODEL_PATH, "\": \n\t", err);
      return false;
    }
    if (warn != "")
    {
      LOG_WARN("A exception occured whilst loading object: \n\t", warn);
    }

    std::unordered_map<Vertex, uint32_t, std::hash<Vertex>, std::equal_to<Vertex>> uniqueVertices;
    uint32_t i = 0;
    for (const auto& shape : shapes)
    {
      for (const auto& index : shape.mesh.indices)
      {
        Vertex vertex {};
        vertex.pos = {
          // -attrib.vertices[3 * index.vertex_index + 0],
          // attrib.vertices[3 * index.vertex_index + 2],
          // attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
        };
        // vertex.pos*=100.0f;
        // vertex.pos.z-=1.5f;
        vertex.texCoord = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };
        
        vertex.color = {1.0f, 1.0f, 1.0f};

        if (uniqueVertices.count(vertex) == 0)
        {
          uniqueVertices[vertex] = static_cast<uint32_t>(pContext->verticies.size());
          pContext->verticies.push_back(vertex);
        }

        pContext->indices.push_back(uniqueVertices[vertex]);
        i++;
        // if (i%3 == 0) LOG_DEBUG(pContext->indices[i-1], ", ", pContext->indices[i-2], ", ", pContext->indices[i-3]);
      }
    }
    LOG_DEBUG("Loaded ", pContext->verticies.size(), " vertices - ", pContext->indices.size()/3, " tris (", pContext->indices.size(), " idc)!");
    return true;
  }
}