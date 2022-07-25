#pragma once
#include "types.hpp"
#include "model.hpp"
#include "layer.hpp"
#include <vector>
#include <unordered_map>


using namespace std;

// A hash function used to hash a pair of any kind
struct hash_pair {
    size_t operator()(const pair<int, int>& p) const
    {
        int hp = (p.first >= p.second) ? (p.first * p.first + p.first + p.second) : (p.second * p.second + p.first);
        return std::hash<int>()(hp);
    }
};

class Grid {
public:
    Grid(tfloat width, tfloat height);
    void add_layer(shared_ptr<Layer> layer);
    void add_model(shared_ptr<Model> model);
    void to_gltf(const string & folder) const;
    void tile_merge();


protected:
    void export_layout(const string & folder) const;


    tfloat width;
    tfloat height;
    unordered_map<pair<int, int>, vector<shared_ptr<Model>>, hash_pair> grid;
};
