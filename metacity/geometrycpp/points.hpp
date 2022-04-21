#pragma once
#include "models.hpp"


class PointCloud : public Model
{
public:
    PointCloud();
    PointCloud(const vector<tvec3> & v);
    PointCloud(const vector<tvec3> && v);

    virtual shared_ptr<Model> copy() const override;
    virtual const char * type() const override;
    virtual size_t to_obj(const string & path, const size_t offset) const override;
};