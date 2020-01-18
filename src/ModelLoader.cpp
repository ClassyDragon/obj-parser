#include "ModelLoader.h"

Model ModelLoader::LoadModel(const std::string& modelFile, const std::string& matDir, const std::string& texDir) {
    std::ifstream objFile(modelFile);
    if (!objFile.good()) {
        std::cout << "Couldn't find obj file: " << modelFile << "." << std::endl;
        return Model();
    }

    /*
     * These containers will be used to store the parsed information in the following format:
     *
     * 1. VertexPositions : Holds any vertex position information marked by the "v" tag
     *      Example:
     *          VertexPositions {
     *              { 1.0, 8.3, 2.1 },
     *              { 5.2, 6.5, 7.3 },
     *              ...
     *              { 0.4, 5.3, 9.2 }
     *          };
     *
     * 2. Normals : Holds any normal vector information marked by the "vn" tag
     *      Example:
     *          Normals {
     *              { 1.0, 8.3, 2.1 },
     *              { 5.2, 6.5, 7.3 },
     *              ...
     *              { 0.4, 5.3, 9.2 }
     *          };
     *
     * 3. TextureCoordinates : Holds any texture coordinate information marked by the "vt" tag
     *      Example:
     *          TextureCoordinates {
     *              { 0.5, 0.5 },
     *              { 0.8, 0.3 },
     *              ...
     *              { 0.6, 0.3 }
     *          };
     *
     * 4. materials : Holds materials and a key to identify them
     *
     * 5. meshIndices : Holds index information for a single mesh, named by the material defining it
     *      Example:
     *          meshIndices {
     *              { "material01", { ... } },
     *              { "material02", { ... } },
     *              ...
     *              { "material0n", { ... } }
     *          };
     */
    std::vector<vec3> VertexPositions, Normals;
    std::vector<vec2> TextureCoordinates;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, std::vector<TriangleIndex>> meshIndices;

    bool newMesh = false;
    std::string input;
    while (objFile >> input) {
        if (newMesh) {
            newMesh = false;
            // Create new mesh using specified material
            std::string matName = input;
            std::vector<TriangleIndex> triIndices;
            while (objFile >> input) {
                if (input.find("usemtl") != std::string::npos) {
                    newMesh = true;
                    break;
                }
                else if (input == "f") {
                    TriangleIndex tri;
                    objFile >> tri.indices[0];
                    objFile >> tri.indices[1];
                    objFile >> tri.indices[2];
                    triIndices.push_back(tri);
                }
            }
            meshIndices.insert(std::pair<std::string, std::vector<TriangleIndex>>(matName, triIndices));
        }
        // Ignore comments and other tags that aren't yet implemented:
        else if (
                   input == "#" 
                || input == "o"
                || input == "s" 
                || input == "g"
                ) 
        {
            objFile.ignore(256, '\n');
        }
        // Read and define materials:
        else if (input.find("mtllib") != std::string::npos) {
            std::string materialFile;
            objFile >> materialFile;
            std::ifstream mtfile(matDir + materialFile);
            if (!mtfile.good()) {
                std::cout << "Failed to open material file." << std::endl;
                return Model();
            }
            while (mtfile >> input) {
                // Ignore lines before a material definition:
                if (input.find("newmtl") == std::string::npos) {
                    mtfile.ignore(256, '\n');
                }
                else {
                    std::string materialName;
                    mtfile >> materialName;
                    while (mtfile >> input) {
                        if (input.find("newmtl") != std::string::npos) {
                        }
                        // "map_Kd" : texture name
                        else if (input.find("map_Kd") != std::string::npos) {
                            mtfile >> input;
                            materials.emplace(std::pair<std::string, Material*>(materialName, new Material(texDir + input)));
                            break;
                        }
                        // Ignore tags that aren't yet implemented:
                        else {
                            mtfile.ignore(256, '\n');
                        }
                    }
                }
            }
            mtfile.close();
        }
        else if (input.find("usemtl") != std::string::npos) {
            // Create new mesh using specified material
            std::string matName;
            objFile >> matName;
            std::vector<TriangleIndex> triIndices;
            while (objFile >> input) {
                if (input.find("usemtl") != std::string::npos) {
                    newMesh = true;
                    break;
                }
                else if (input == "f") {
                    /* struct TriangleIndex {
                     *      std::string indices[3];     
                     * };
                     */
                    TriangleIndex tri;
                    objFile >> tri.indices[0];
                    objFile >> tri.indices[1];
                    objFile >> tri.indices[2];
                    triIndices.push_back(tri);
                }
            }
            meshIndices.insert(std::pair<std::string, std::vector<TriangleIndex>>(matName, triIndices));
        }
        else if (input.find("vt") != std::string::npos) {
            // Read vertex texture coordinates
            float xy[2];
            for (int i = 0; i < 2; i++) {
                objFile >> xy[i];
            }
            vec2 texCoords(xy[0], xy[1]);
            TextureCoordinates.push_back(texCoords);
        }
        else if (input.find("vn") != std::string::npos) {
            // Read vertex normal
            float xyz[3];
            for (int i = 0; i < 3; i++) {
                objFile >> xyz[i];
            }
            vec3 normalValues(xyz[0], xyz[1], xyz[2]);
            Normals.push_back(normalValues);
        }
        else if (input.find("v") != std::string::npos) {
            // Read vertex positions
            float xyz[3];
            for (int i = 0; i < 3; i++) {
                objFile >> xyz[i];
            }
            vec3 vertex(xyz[0], xyz[1], xyz[2]);
            VertexPositions.push_back(vertex);
        }
    }
    objFile.close();

    // At this point:
    // meshIndices contains mesh data in <material name, indices> format.
    // VertexPositions contains vec3's of all vertex positions.
    // TextureCoordinates contains vec2's of all texture coordinates.
    // Normals contains vec3's of all normal vectors.

    // Convert the indices:
    std::vector<Vertex> vertices;
    std::unordered_map<std::string, unsigned int> indices;
    std::vector<Mesh> meshes;
    unsigned int vertexNum = 0;
    // Iterate through all indices and construct unique vertices for all of them.
    /*
        std::unordered_map<std::string, std::vector<TriangleIndex>> meshIndices;
    */
    for (auto& mesh : meshIndices) { // for each vector in map
        // New mesh to be added to the model:
        Mesh m;

        // Indices converted from v/vt/vn to i format
        std::vector<unsigned int> meshConvertedIndices;

        for (auto& triangle : mesh.second) { // for each TriangleIndex(string, string, string) in vector
            for (auto& index : triangle.indices) { // for each std::string in TriangleIndex

                // If index is new, create a corresponding vertex:
                if (indices.find(index) == indices.end()) {
                    // Deconstruct string in v/vt/vn format
                    std::string vertexData = index;
                    for (auto& c : vertexData) {
                        if (c == '/')
                            c = ' ';
                    }

                    // Create vertex using converted string
                    std::stringstream ss(vertexData);
                    unsigned int vertexAttributes[3];
                    for (int i = 0; i < 3; i++) {
                        ss >> vertexAttributes[i];
                    }
                    vertices.push_back(Vertex {
                            VertexPositions[vertexAttributes[0] - 1].x,
                            VertexPositions[vertexAttributes[0] - 1].y,
                            VertexPositions[vertexAttributes[0] - 1].z,
                            TextureCoordinates[vertexAttributes[1] - 1].x,
                            TextureCoordinates[vertexAttributes[1] - 1].y,
                            Normals[vertexAttributes[2] - 1].x,
                            Normals[vertexAttributes[2] - 1].y,
                            Normals[vertexAttributes[2] - 1].z
                            });

                    // Create unique index in map
                    // Example: indices<"1/1/1", 5>
                    indices[index] = vertexNum;
                    vertexNum++;
                }

                // Add converted index to the meshes vector of indices:
                meshConvertedIndices.push_back(indices[index]);
            }
        }
        // meshConvertedIndices example:
        // { 
        //      0, 1, 2,
        //      3, 4, 5
        // };
        m.m_Indices = meshConvertedIndices;
        meshes.push_back(m);
    }

    // Set completed VertexBuffer to all meshes:
    for (auto& mesh : meshes) {
        mesh.m_Vertices = vertices;
    }

    Model model;
    model.meshes = meshes;
    return model;
}
