#include "simplify.hpp"
#include "graham.hpp"
#include "bvh.hpp"
#include "progress.hpp"

namespace simplify
{
    shared_ptr<Model> simplify_envelope(const shared_ptr<Model> model)
    {
        const auto attribute = model->get_attribute("POSITION");
        if (attribute == nullptr)
        {
            return nullptr;
        }

        if (attribute->type() != AttributeType::POLYGON)
        {
            return model->clone();
        }

        vector<tvec2> projected;
        for (int i = 0; i < attribute->size(); i++)
        {
            projected.emplace_back((*attribute)[i]);
        }

        const auto bbox = attribute->bbox();
        const auto polygon = grahamScan(projected);
        const auto new_attribute = make_shared<Attribute>();

        vector<vector<tfloat>> npolygon;
        vector<tfloat> bottom;

        for (const auto &point : polygon)
        {
            bottom.push_back(point.x);
            bottom.push_back(point.y);
            bottom.push_back(bbox.first.z);
        }

        npolygon.emplace_back(move(bottom));
        new_attribute->push_polygon3D(npolygon);
        npolygon.clear();

        vector<tfloat> top;
        for (const auto &point : polygon)
        {
            top.push_back(point.x);
            top.push_back(point.y);
            top.push_back(bbox.second.z);
        }
        npolygon.emplace_back(move(top));
        new_attribute->push_polygon3D(npolygon);
        npolygon.clear();

        vector<tvec3> sides;
        for (int i = 0; i < polygon.size(); i++)
        {
            sides.emplace_back(tvec3(polygon[i].x, polygon[i].y, bbox.first.z));
            sides.emplace_back(tvec3(polygon[(i + 1) % polygon.size()].x, polygon[(i + 1) % polygon.size()].y, bbox.first.z));
            sides.emplace_back(tvec3(polygon[i].x, polygon[i].y, bbox.second.z));
            sides.emplace_back(tvec3(polygon[i].x, polygon[i].y, bbox.second.z));
            sides.emplace_back(tvec3(polygon[(i + 1) % polygon.size()].x, polygon[(i + 1) % polygon.size()].y, bbox.first.z));
            sides.emplace_back(tvec3(polygon[(i + 1) % polygon.size()].x, polygon[(i + 1) % polygon.size()].y, bbox.second.z));
        }

        new_attribute->push_triangles(sides);

        auto simple_model = make_shared<Model>();
        simple_model->add_attribute("POSITION", new_attribute);
        simple_model->set_metadata(model->get_metadata());
        return simple_model;
    }

    shared_ptr<Model> merge_models(const vector<shared_ptr<Model>> &models)
    {
        if (models.size() == 0)
            return make_shared<Model>();

        shared_ptr<Model> model = models[0]->clone();

        for (int i = 1; i < models.size(); i++)
        {
            model->merge(models[i]);
        }
        return model;
    }

    void simplify_remesh_height(vector<shared_ptr<Model>> &models, const tfloat tile_side, const size_t tile_divisions)
    {
        Progress bar("Remeshing height");
        auto bvh = BVH(models);
        if (bvh.is_empty()) {
            models.clear();
            return;
        }

        const auto bbox = bvh.bbox();
        const tfloat step = tile_side / tile_divisions;

        vector<vector<tvec3>> vertices;
        for (tfloat x = bbox.min.x; x <= bbox.max.x; x += step){
            vector<tvec3> row;
            for (tfloat y = bbox.min.y; y <= bbox.max.y; y += step)
            {
                bar.update();
                row.emplace_back(tvec3(x, y, bvh.traceDownRegualarRay(x, y, bbox.min.z)));
            }
            vertices.emplace_back(move(row));
        }

        //todo smoothing and removing infinities

        const size_t count_tiles_x = ceil((bbox.max.x - bbox.min.x) / tile_side);
        const size_t count_tiles_y = ceil((bbox.max.y - bbox.min.y) / tile_side);
        size_t x_start,  y_start, x_end, y_end;
        vector<tvec3> tile_triangles;
        vector<shared_ptr<Model>> new_models;

        for (size_t i = 0; i < count_tiles_x; i++)
            for (size_t j = 0; j < count_tiles_y; j++)
            {
                tile_triangles.clear();
                x_start = i * tile_divisions;
                y_start = j * tile_divisions;
                x_end = min(x_start + tile_divisions, vertices.size());
                y_end = min(y_start + tile_divisions, vertices[0].size());

                for (size_t x = x_start; x < x_end; x++)
                    for (size_t y = y_start; y < y_end; y++)
                    {
                        tile_triangles.emplace_back(vertices[x][y]);
                        tile_triangles.emplace_back(vertices[x][y + 1]);
                        tile_triangles.emplace_back(vertices[x + 1][y]);
                        tile_triangles.emplace_back(vertices[x + 1][y]);
                        tile_triangles.emplace_back(vertices[x][y + 1]);
                        tile_triangles.emplace_back(vertices[x + 1][y + 1]);
                    }

                auto new_model = make_shared<Model>();
                auto new_attribute = make_shared<Attribute>();
                new_attribute->push_triangles(tile_triangles);
                new_model->add_attribute("POSITION", new_attribute);
                new_models.emplace_back(new_model);
            }

        models = move(new_models);
    }

}