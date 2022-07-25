#include <stdexcept>
#include <numeric>
#include "model.hpp"
#include "triangulation.hpp"
#include "cppcodec/base64_rfc4648.hpp"
#include "gltf/tiny_gltf.h"
#include "convert.hpp"

//===============================================================================
Model::Model() {}


void Model::merge(shared_ptr<Model> model)
{
    for (auto & pair : model->attrib) {
        if (attrib.find(pair.first) == attrib.end()) {
            throw runtime_error("Merging Failed: Attribute " + pair.first + " found in RModel does not exist in LModel");
        } else {
            attrib[pair.first]->merge(pair.second);
        }
    }

    /*if (metadata.is_array()) {
        metadata.push_back(model->metadata);
    } else {
        metadata = { metadata, model->metadata };
    }*/
}

shared_ptr<Model> Model::clone() const
{
    auto clone = make_shared<Model>();
    for (auto & pair : attrib) {
        clone->attrib[pair.first] = pair.second->clone();
    }
    clone->metadata = metadata;
    return clone;
}

void Model::add_attribute(const string &name, shared_ptr<Attribute> attribute) {
    if (attrib.find(name) != attrib.end()) {
        throw runtime_error("Attribute already exists");
    }
    attrib[name] = attribute;
}

tvec3 Model::get_centroid() const
{
    if (attrib.find("POSITION") == attrib.end()) {
        throw runtime_error("No position data");
    }  

    const auto positions = attrib.at("POSITION");
    auto centroid = positions->sum();
    centroid /= positions->size();
    return centroid;
}

void Model::set_metadata(const nlohmann::json & data)
{
    for (auto & pair : data.items()) {
        metadata[pair.key()] = pair.value();
    }
}

nlohmann::json Model::get_metadata() const
{
    return metadata;
}


shared_ptr<Attribute> Model::get_attribute(const string &name) const {
    if (attrib.find(name) == attrib.end()) {
        //throw runtime_error("Attribute does not exist");
        return nullptr;
    }
    return attrib.at(name);
}

bool Model::attribute_exists(const string &name) {
    return attrib.find(name) != attrib.end();
}


void Model::compute_normals()
{
    //TODO
}

bool Model::has_any_geometry() const
{
    if (attrib.find("POSITION") == attrib.end()) {
        return false;
    }

    if (attrib.at("POSITION")->size() < 1) {
        return false;
    }

    return true;
}

void Model::to_gltf(tinygltf::Model & model) const
{
    int mesh_index;

    if (!has_any_geometry()) {
        return;
    }

    to_gltf_mesh(model, mesh_index);
    to_gltf_scene(model, mesh_index);
}

void Model::to_gltf_scene(tinygltf::Model & model, const int mesh_index) const
{
    if (model.scenes.size() == 0) {
        tinygltf::Scene scene;
        model.scenes.push_back(scene);
        model.defaultScene = 0;
    }

    int node_index;
    to_gltf_node(model, mesh_index, node_index);
    model.scenes[0].nodes.push_back(node_index);
}

void Model::to_gltf_node(tinygltf::Model & model, const int mesh_index, int & node_index) const
{
    tinygltf::Node node;
    node.mesh = mesh_index;
    model.nodes.push_back(node);
    node_index = model.nodes.size() - 1;
}

void Model::to_gltf_mesh(tinygltf::Model & model, int & mesh_index) const
{
    tinygltf::Mesh mesh;
    to_gltf_primitive(model, mesh);
    mesh.extras = to_gltf_value(metadata);
    model.meshes.push_back(mesh);
    mesh_index = model.meshes.size() - 1;
}

int type_to_gltf(AttributeType type)
{
    switch (type)
    {
    case AttributeType::POINT:
        return TINYGLTF_MODE_POINTS;
    case AttributeType::SEGMENT:
        return TINYGLTF_MODE_LINE;
    case AttributeType::POLYGON:
        return TINYGLTF_MODE_TRIANGLES;
    default:
        throw runtime_error("Undefined attribute type");
    }
}

void Model::to_gltf_primitive(tinygltf::Model & model, tinygltf::Mesh & mesh) const
{
    tinygltf::Primitive primitive;
    to_gltf_attribute(model, primitive, "POSITION");
    //to_gltf_attribute(model, primitive, "NORMAL");
    primitive.indices = -1; //no indices
    mesh.primitives.push_back(primitive);
}

void Model::to_gltf_attribute(tinygltf::Model & model, tinygltf::Primitive & primitive, const string &name) const
{
    int accessor_index;
    AttributeType type;
    shared_ptr<Attribute> position_attribute = get_attribute(name);
    position_attribute->to_gltf(model, type, accessor_index);

    if (name == "POSITION") {
        primitive.mode = type_to_gltf(type);
    } 

    primitive.attributes[name] = accessor_index;
}

//===============================================================================


AttributeType type_from_gltf(int mode)
{
    switch (mode)
    {
    case TINYGLTF_MODE_POINTS:
        return AttributeType::POINT;
        break;
    case TINYGLTF_MODE_LINE:
        return AttributeType::SEGMENT;
        break;
    case TINYGLTF_MODE_TRIANGLES:
        return AttributeType::POLYGON;
        break;
    default:
        throw runtime_error("Undefined attribute type");
    }
}


void Model::from_gltf(const tinygltf::Model & model, const int mesh_index)
{
    mesh_validity_check(model, mesh_index);
    const tinygltf::Mesh & mesh = model.meshes[mesh_index];
    metadata = to_json_value(model.meshes[mesh_index].extras);
    const tinygltf::Primitive & primitive = mesh.primitives[0];
    from_gltf_attribute(model, primitive, "POSITION", type_from_gltf(primitive.mode));
}

void Model::from_gltf_attribute(const tinygltf::Model & model, const tinygltf::Primitive & primitive, const string &name, AttributeType type)
{
    if (primitive.attributes.find(name) == primitive.attributes.end()) {
        //throw runtime_error("Attribute does not exist");
        return;
    }

    int attribute_index = primitive.attributes.at(name);
    shared_ptr<Attribute> attribute = make_shared<Attribute>();
    attribute->from_gltf(model, type, attribute_index);
    add_attribute(name, attribute);
}


//===============================================================================
// Checks

void Model::mesh_validity_check(const tinygltf::Model & model, const int mesh_index)
{
    if (mesh_index >= model.meshes.size()) {
        throw runtime_error("Mesh index out of range");
    }

    if (model.meshes[mesh_index].primitives.size() != 1) {
        throw runtime_error("Only one primitive per mesh is supported");
    }

    if (model.meshes[mesh_index].primitives[0].indices != -1) {
        throw runtime_error("Indices are not supported");
    }

    if (model.meshes[mesh_index].primitives[0].attributes.size() != 1) {
        throw runtime_error("Only one attribute per primitive is supported");
    }
}


//===============================================================================

shared_ptr<Model> merge_models(vector<shared_ptr<Model>> models)
{
    if (models.size() == 0)
        return make_shared<Model>();

    shared_ptr<Model> model = models[0]->clone();

    for (int i = 1; i < models.size(); i++) {
        model->merge(models[i]);
    }
    return model;
}