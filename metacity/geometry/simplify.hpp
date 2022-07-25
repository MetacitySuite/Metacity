#pragma once
#include "model.hpp"


namespace simplify {
    shared_ptr<Model> simplify_envelope(const shared_ptr<Model> model);
    shared_ptr<Model> merge_models(const vector<shared_ptr<Model>> & models);
    void simplify_remesh_height(vector<shared_ptr<Model>> & models, const tfloat tile_side, const size_t tile_divisions);
}