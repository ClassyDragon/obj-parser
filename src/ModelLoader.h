#pragma once

/*
 * --------------------------------------------------------------------------------------------------
 *
 *                                    ModelLoader / OBJ Parser
 *                  Written by Caleb Geyer : http://www.github.com/ClassyDragon/
 *
 * --------------------------------------------------------------------------------------------------
 * Object (.obj) files are a 3D model format that can be
 * generated by programs such as Blender, along with their
 * associated Material (.mat) file. The format is very close
 * to what would typically be used with a graphics rendering
 * API such as the one used in OpenGL. This static model
 * loading function reads the necessary information for creating
 * the necessary memory structures for rendering a 3D model:
 * 1. Vertices
 *      - Includes:
 *      1. Position
 *      2. Texture Coordinates
 *      3. Normal Vectors
 * 2. Indices
 * 3. Materials
 *      - Includes:
 *      1. Texture file
 * This data is returned in its literal form to be used in
 * whatever way the user decides.
 *
 * --------------------------------------------------------------------------------------------------
 *
 *                                               USE
 *
 * // Materials and Textures in specified directories:
 * Model modelVar = ModelLoader::LoadModel("res/models/model.obj", "res/materials/", "res/textures");
 *
 * // Materials and Textures in current directory:
 * Model modelVar = ModelLoader::LoadModel("model.obj");
 *
 * --------------------------------------------------------------------------------------------------
 *
 *                              IMPORANT NOTES WHEN GENERATING OBJ FILES
 *
 * 1. Be sure to triangulate the faces.
 * 2. Be sure to generate Normal data.
 *
 * --------------------------------------------------------------------------------------------------
 *
 *                                           BUGS / TODO
 *
 * 1. The format of acceptable data is fairly rigid. However,
 *      exporting from Blender should generate a file in the
 *      correct format.
 * 2. If multiple meshes share a material, the original will
 *      be overwritten.
 *
 * --------------------------------------------------------------------------------------------------
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>

class ModelLoader;
class Model;
class Material;

struct vec3 {
    float x, y, z;
    vec3(float x, float y, float z)
        : x(x), y(y), z(z)
    {
    }
};

struct vec2 {
    float x, y;
    vec2(float x, float y)
        : x(x), y(y)
    {
    }
};

struct Vertex {
    float x, y, z, texW, texH, nX, nY, nZ;
};

struct TriangleIndex {
    std::string indices[3];
};

class Mesh {
    public:
        std::vector<Vertex> m_Vertices;       // Vertex Buffer Object
        std::vector<unsigned int> m_Indices; // Index Buffer Object
        Material* m_Material;
        Mesh()
        {
        }
        ~Mesh() {
        }
};

class Model {
    public:
        std::vector<Mesh> meshes;
        Model()
        {
        }
        ~Model() {
        }
};

class Material {
    public:
        std::string texFile;
        Material(const std::string& texFile) {
            this->texFile = texFile;
        }
};

class ModelLoader {
    public:
        static Model LoadModel(const std::string& modelFile, const std::string& matDir = "", const std::string& texDir = "");
};
