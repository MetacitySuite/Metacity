#pragma once
#include "types.hpp"
#include "model.hpp"
#include "bbox.hpp"

using namespace std;

enum BVHNodeType
{
    x = 0,
    y = 1,
    z = 2,
    leaf = 3
};

struct BVHNode
{
    BBox bbox;
    uint8_t type;
};

struct Triangle;

struct Ray
{
    tvec3 origin;
    tvec3 dir;
    tfloat t;
};

#define RTINFINITY FLT_MAX
#define RTEPSILON 3.402823e-15 * 2


bool intersects(const BBox &b, Ray & ray, float & min_t, float & max_t);

struct Triangle
{
    Triangle(tvec3 _a, tvec3 _b, tvec3 _c)
    : a(_a), b(_b), c(_c), e1(_b - _a), e2(_c - _a)
    {}

    // Möller-Trumbore algorithm
    // Find intersection point - from PBRT - www.pbrt.org
    // the source is modified, inspired by above
    tfloat intersect(Ray &ray)
    {
        tfloat b1, b2;
        tvec3 pvec = glm::cross(ray.dir, e2);
        tfloat det = glm::dot(e1, pvec);

        if (fabs(det) < RTEPSILON) // ray is parallel to triangle
            return RTINFINITY;

        tfloat invDet = 1.0f / det;

        // Compute first barycentric coordinate
        tvec3 tvec = ray.origin - a;
        b1 = dot(tvec, pvec) * invDet;

        if (b1 < 0.0f || b1 > 1.0f)
            return RTINFINITY;

        // Compute second barycentric coordinate
        tvec3 qvec = cross(tvec, e1);
        b2 = dot(ray.dir, qvec) * invDet;

        if (b2 < 0.0f || b1 + b2 > 1.0f)
            return RTINFINITY;

        // Compute t to intersection point
        tfloat t = dot(e2, qvec) * invDet;
        return t;
    }

    tvec3 a, b, c;
    tvec3 e1, e2;
};

struct BVHLeafNode : public BVHNode
{
    BVHLeafNode(tvec3 _a, tvec3 _b, tvec3 _c)
    : triangle(_a, _b, _c) {}

    Triangle triangle;
};

struct BVHInternalNode : public BVHNode
{
    shared_ptr<BVHNode> L;
    shared_ptr<BVHNode> R;
};

class BVH
{
public:
    BVH(const vector<shared_ptr<Model>> & models_);
    tfloat traceDownRegualarRay(const tfloat x, const tfloat y, const tfloat z) const;
    inline BBox bbox() const { if (root) return root->bbox; return empty_bbox(); };
    inline bool is_empty() const { return root == nullptr; }

protected:

    shared_ptr<BVHNode> build(const BBox &box, const size_t start, const size_t end, const uint8_t axis);
    shared_ptr<BVHNode> build_two_nodes(const BBox &box, const size_t start, const size_t end, const uint8_t axis);
    shared_ptr<BVHNode> build_general(const BBox &box, const size_t start, const size_t end, const uint8_t axis);
    size_t classify(const tfloat split, BBox &left, BBox &right, const size_t start, const size_t end, const uint8_t axis);
    size_t handle_special_case(BBox &left, BBox &right, const size_t start, const size_t end, const uint8_t axis);

    void recursiveTrace(Ray & ray, shared_ptr<BVHNode> node) const;

    vector<shared_ptr<Model>> models;
    shared_ptr<BVHNode> root;
    vector<shared_ptr<BVHNode>> nodes;
};
