#include "layer.hpp"
#include "bvh.hpp"
#include "gltf/tiny_gltf.h"
#include "progress.hpp"
#include "mapping.hpp"
#include "simplify.hpp"


Layer::Layer() {}

void Layer::add_model(shared_ptr<Model> model) {
    models.push_back(model);
}

void Layer::add_models(const vector<shared_ptr<Model>> & models) {
    this->models.insert(this->models.end(), models.begin(), models.end());
}

const vector<shared_ptr<Model>> & Layer::get_models() const {
    return models;
}

void Layer::map_to_height(shared_ptr<Layer> height_layer) {
    auto bvh = BVH(height_layer->get_models());
    to_height(bvh, models);
}

void Layer::to_gltf(const string &filename) const {
    tinygltf::Model gltf_model;
    tinygltf::Asset asset;
    asset.version = "2.0";
    asset.generator = "Metacity";
    gltf_model.asset = asset;
    
    Progress bar("Exporting models");
    for (auto model : models) {
        bar.update();
        model->to_gltf(gltf_model);
    }

    tinygltf::TinyGLTF gltf;
    //gltf.SetStoreOriginalJSONForExtrasAndExtensions(true);
    gltf.WriteGltfSceneToFile(&gltf_model, filename, true, true, true, false);
}

void Layer::simplify_envelope()
{
    Progress bar("Simplifying envelope");
    for (auto & model : models) {
        bar.update();
        model = simplify::simplify_envelope(model);
    }
}

void Layer::simplify_remesh_height(tfloat tile_side, size_t tile_divisions)
{
    simplify::simplify_remesh_height(models, tile_side, tile_divisions);
}

void Layer::from_gltf(const string &filename) {
    tinygltf::TinyGLTF gltf;
    //gltf.SetStoreOriginalJSONForExtrasAndExtensions(true);
    tinygltf::Model gltf_model;
    string err, warn;

    Progress bar("Importing models");
    bool ret = gltf.LoadASCIIFromFile(&gltf_model, &err, &warn, filename);

    if (!err.empty()) {
        cout << err << endl;
    }

    if (!warn.empty()) {
        cout << warn << endl;
    }

    if (!ret) {
        cout << "Failed to load gltf file" << endl;
        return;
    }

    for (int mesh_idx = 0; mesh_idx < gltf_model.meshes.size(); mesh_idx++) {
        bar.update();
        auto model = make_shared<Model>();
        model->from_gltf(gltf_model, mesh_idx);
        add_model(model);
    } 
}