#include <fstream>
#include "grid.hpp"
#include "progress.hpp"
#include "simplify.hpp"
#include "gltf/json.hpp"


Grid::Grid(tfloat _width, tfloat _height) : width(_width), height(_height) {}

void Grid::add_layer(shared_ptr<Layer> layer) {
    Progress bar("Creating grid");
    for (auto model : layer->get_models()) {
        bar.update();
        add_model(model);
    }
}

void Grid::add_model(shared_ptr<Model> model) 
{
    tvec3 centroid = model->get_centroid();
    int x = (int) floor(centroid.x / width);
    int y = (int) floor(centroid.y / height);
    
    pair<int, int> key = make_pair(x, y);
    if (grid.find(key) == grid.end()) {
        grid[key] = vector<shared_ptr<Model>>();
    }
    grid[key].push_back(model);
}

void Grid::tile_merge()
{
    Progress bar("Merging tiles");
    for (auto & pair : grid) {
        bar.update();
        auto models = pair.second;
        auto merged = simplify::merge_models(models);
        pair.second.clear();
        pair.second.push_back(merged);
    }
}

void Grid::to_gltf(const string & folder) const
{
    Progress bar("Exporting grid");
    for (auto & pair : grid) {
        bar.update();

        string filename = folder + "/tile" + to_string(pair.first.first) + "_" + to_string(pair.first.second) + ".glb";

        tinygltf::Model gltf_model;
        tinygltf::Asset asset;
        asset.version = "2.0";
        asset.generator = "Metacity";
        gltf_model.asset = asset;

        for (auto & model : pair.second) {
            model->to_gltf(gltf_model);
        }
        
        tinygltf::TinyGLTF gltf;
        gltf.SetStoreOriginalJSONForExtrasAndExtensions(true);
        gltf.WriteGltfSceneToFile(&gltf_model, filename, true, true, true, true);
    }

    export_layout(folder);
}

void Grid::export_layout(const string & folder) const
{
    nlohmann::json layout;
    layout["tileWidth"] = width;
    layout["tileHeight"] = height;
    layout["tiles"] = nlohmann::json::array();
    
    for (auto & pair : grid) {
        nlohmann::json tile;
        tile["x"] = pair.first.first;
        tile["y"] = pair.first.second;
        tile["file"] = "tile" + to_string(pair.first.first) + "_" + to_string(pair.first.second) + ".glb";
        tile["size"] = pair.second.size();
        layout["tiles"].push_back(tile);
    }

    ofstream file(folder + "/layout.json");
    file << layout.dump(4);
    file.close();
}
