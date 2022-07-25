#include "mapping.hpp"
#include "progress.hpp"


void to_height(BVH & bvh, vector<shared_ptr<Model>> & models) {
    Progress bar("Height mapping");
    for (auto & model : models) {
        bar.update();

        auto attr = model->get_attribute("POSITION");
        if (!attr)
            continue;

        tvec3 pos;
        tfloat h;
        for (size_t i = 0; i < attr->size(); i++) {
            pos = (*attr)[i];
            h = bvh.traceDownRegualarRay(pos.x, pos.y, pos.z);
            pos.z = h;
            (*attr)[i] = pos;
        }
    }
}