#pragma once

#include "model.hpp"

//std::string obj_file = (std::string)Project_SOURCE_DIR + "/src/assets/LowpolyForestPack/low_poly_tree_1.obj";
//std::string material_directory = (std::string)Project_SOURCE_DIR + "/src/assets/LowpolyForestPack";

class PineTree: public Model{
    private:
        std::vector<float> setupVertices(std::vector<glm::vec3> model_vertices,std::vector<glm::vec3> model_normals);
        void setupTreeData(std::vector<glm::vec3> model_vertices,std::vector<glm::vec3> model_normals,std::vector<float>& vertices,std::vector<float>& normals,std::vector<int>& material_ids);
        void set_instance_buffer(std::vector<glm::vec3>& offsets);
        void set_instance_matrix_buffer(std::vector<glm::mat4>& instanceMatrices);
        void calculate_TreeOffsets();
        glm::vec2 translations[100];
        

    public:
        PineTree();
        void draw(int width, int height) override;
        void draw_instanced(int width,int height,std::vector<glm::mat4> instance_matrices, glm::vec3& planet_info);
        void draw_for_depth_map(std::vector<glm::mat4> instance_matrices, glm::vec3& planet_info);
};