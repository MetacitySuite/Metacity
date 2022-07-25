#pragma once
#include "types.hpp"
#include "model.hpp"
using namespace std;

class Layer {
public:
    Layer();
    void add_model(shared_ptr<Model> model);
    void add_models(const vector<shared_ptr<Model>> & models);
    
    void map_to_height(shared_ptr<Layer> height_layer);

    const vector<shared_ptr<Model>> & get_models() const;
    void to_gltf(const string &filename) const;
    void from_gltf(const string &filename);
    
    int size() const {
        return models.size();
    }

    void simplify_envelope();
    void simplify_remesh_height(tfloat tile_side, size_t tile_divisions);

protected:
    vector<shared_ptr<Model>> models;
};