#include "ModelLoader.h"

int main() {
    Model m = ModelLoader::LoadModel("blue.obj");
    std::cout << "Model file: blue.obj" << std::endl;
    std::cout << "Number of vertices: " << m.meshes[0].m_Vertices.size() << std::endl;
    std::cout << "Indices for each mesh: " << std::endl;
    for (auto& mesh : m.meshes) {
        for (auto& index : mesh.m_Indices) {
            std::cout << index << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}
