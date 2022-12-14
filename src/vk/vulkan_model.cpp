#include "vulkan_base.h"
#include "../pch.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Ths::Vk
{
  bool loadModel([[maybe_unused]] VContext* pContext, MeshResource*& pMesh, const char* modelpath, const char* basedir)
  {
    pMesh = new MeshResource();
    return loadModel(pContext, *pMesh, modelpath, basedir);
  }
  bool loadModel([[maybe_unused]] VContext* pContext, MeshResource& mesh, const char* modelpath, const char* basedir)
  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // std::string basedir = "assets/models/";
    // std::string basedir = BASE_PATH + "assets/models";
    // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), basedir.c_str(), true, true))
    std::string compoundPath;
    compoundPath += basedir;
    compoundPath += modelpath;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, compoundPath.c_str(), basedir, true, true))
    {
      LOG_ERROR("An error occured whilst loading \"", modelpath, "\": \n\t", err);
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
          uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
          mesh.vertices.push_back(vertex);
        }

        mesh.indices.push_back(uniqueVertices[vertex]);
        i++;
        // if (i%3 == 0) LOG_DEBUG(pContext->indices[i-1], ", ", pContext->indices[i-2], ", ", pContext->indices[i-3]);
      }
    }
    LOG_DEBUG("Loaded ", mesh.vertices.size(), " vertices - ", mesh.indices.size()/3, " tris (", mesh.indices.size(), " idc)!");
    return true;
  }
}