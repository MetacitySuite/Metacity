#include <stdexcept>
#include <fstream>
#include <iomanip>
#include "mesh.hpp"
#include "triangulation.hpp"

//===============================================================================

Mesh::Mesh() : Model() {}
Mesh::Mesh(const vector<tvec3> &v) : Model(v) {}
Mesh::Mesh(const vector<tvec3> &&v) : Model(move(v)) {}

shared_ptr<Model> Mesh::copy() const
{
    auto cp = make_shared<Mesh>();
    copy_to(cp);
    return cp;
}

const char *Mesh::type() const
{
    return "mesh";
}

inline tvec3 tcentroid(const tvec3 triangle[3])
{
    tvec3 c = triangle[0] + triangle[1] + triangle[2];
    c /= 3.0;
    return c;
}

const tvec3 * Mesh::triangle(const size_t index) const
{
    return &(vertices[index * 3]);
}

const shared_ptr<Attribute> Mesh::attribute(const string & name)
{
    const auto it = attrib.find(name);
    if (it == attrib.end())
        throw runtime_error("The model is missing attribute " + name);
    return it->second;
}
