#include "bvh.hpp"

bool intersects(const BBox &b, Ray &ray, tfloat &min_t, tfloat &max_t)
{
    tfloat tmin, tmax;

    tvec3 invDir = tvec3(1.0) / ray.dir;

    tmin = (b.min.x - ray.origin.x) * invDir.x;
    tmax = (b.max.x - ray.origin.x) * invDir.x;

    if (tmin > tmax)
        swap(tmin, tmax);

    tfloat tymin, tymax;

    tymin = (b.min.y - ray.origin.y) * invDir.y;
    tymax = (b.max.y - ray.origin.y) * invDir.y;

    if (tymin > tymax)
        swap(tymin, tymax);

    if ((tmax < tymin) || (tmin > tymax))
        return false;

    if (tymax < tmax)
        tmax = tymax;

    if (tmin < tymin)
        tmin = tymin;

    tfloat tzmin, tzmax;

    tzmin = (b.min.z - ray.origin.z) * invDir.z;
    tzmax = (b.max.z - ray.origin.z) * invDir.z;

    if (tzmin > tzmax)
        swap(tzmin, tzmax);

    if ((tmax < tzmin) || (tmin > tzmax))
        return false;

    if (tzmax < tmax)
        tmax = tzmax;

    if (tmin < tzmin)
        tmin = tzmin;

    min_t = tmin;
    max_t = tmax;
    return true;
}

//===============================================================================

BVH::BVH(const vector<shared_ptr<Model>> & models_)
{

    BBox main;
    set_empty(main);

    for (auto & model : models_)
    {
        const auto attr = model->get_attribute("POSITION");
        
        if (!attr)
            continue;

        if (attr->type() != AttributeType::POLYGON)
            continue;
        
        //iterate over triangles
        for (size_t i = 0, j = 0; i < attr->size(); i += 3, ++j)
        {
            auto node = make_shared<BVHLeafNode>((*attr)[i], (*attr)[i + 1], (*attr)[i + 2]);
            for_triangle(&((*attr)[i]), node->bbox);
            extend(main, node->bbox);
            node->type = BVHNodeType::leaf;
            nodes.push_back(node);
        }

    }

    root = build(main, 0, nodes.size(), 0);
}

shared_ptr<BVHNode> BVH::build_two_nodes(const BBox &box, const size_t start, const size_t end, const uint8_t axis)
{
    auto node = make_shared<BVHInternalNode>();
    node->type = axis;
    node->bbox = box;
    const auto l = nodes[start];
    const auto r = nodes[start + 1];
    for_bboxes(l->bbox, r->bbox, node->bbox);
    const tvec3 lc = l->bbox.centroid();
    const tvec3 rc = r->bbox.centroid();

    if (lc[axis] < rc[axis])
    {
        node->L = l;
        node->R = r;
    }
    else
    {
        node->L = r;
        node->R = l;
    }
    return node;
}

size_t BVH::classify(const tfloat split, BBox &left, BBox &right, const size_t start, const size_t end, const uint8_t axis)
{
    set_empty(left);
    set_empty(right);

    tfloat mid;
    size_t firstRight = start;
    for (size_t i = start; i < end; i++)
    {
        mid = midpoint(nodes[i]->bbox, axis);
        if (mid >= split)
        {
            extend(right, nodes[i]->bbox);
        }
        else
        {
            extend(left, nodes[i]->bbox);
            swap(nodes[i], nodes[firstRight]);
            ++firstRight;
        }
    }

    return firstRight;
}

size_t BVH::handle_special_case(BBox &left, BBox &right, const size_t start, const size_t end, const uint8_t axis)
{
    set_empty(left);
    set_empty(right);

    tfloat minMid = FLT_MAX;
    tfloat mid;

    for (size_t i = start; i < end; i++)
    {
        mid = midpoint(nodes[i]->bbox, axis);
        if (mid < minMid)
            minMid = mid, swap(nodes[i], nodes[start]);
    }

    extend(left, nodes[start]->bbox);
    for (size_t i = start + 1; i < end; i++)
        extend(right, nodes[i]->bbox);

    return start + 1;
}

shared_ptr<BVHNode> BVH::build_general(const BBox &box, const size_t start, const size_t end, const uint8_t axis)
{
    auto node = make_shared<BVHInternalNode>();
    node->bbox = box;
    BBox left, right;
    tfloat split = midpoint(box, axis);
    size_t firstRight = classify(split, left, right, start, end, axis);

    // maybe?
    if (start == firstRight || end == firstRight)
        firstRight = handle_special_case(left, right, start, end, axis);

    uint8_t naxis = (axis + 1) % 2;
    node->L = build(left, start, firstRight, naxis);
    node->R = build(right, firstRight, end, naxis);
    return node;
}

shared_ptr<BVHNode> BVH::build(const BBox &box, const size_t start, const size_t end, const uint8_t axis)
{
    size_t size = end - start;
    if (size == 0)
        return nullptr;
    if (size == 1)
        return nodes[start];
    if (size == 2)
        return build_two_nodes(box, start, end, axis);

    return build_general(box, start, end, axis);
}

tfloat BVH::traceDownRegualarRay(const tfloat x, const tfloat y, const tfloat z) const
{
    // iteration vs recursion
    Ray ray;
    ray.origin = tvec3(x, y, 10000);
    ray.dir = tvec3(0, 0, -1);
    ray.t = RTINFINITY;

    if (root == nullptr)
        return ray.t;

    recursiveTrace(ray, root);
    return 10000 - ray.t;
}

void BVH::recursiveTrace(Ray &ray, shared_ptr<BVHNode> node) const
{
    if (node->type == BVHNodeType::leaf)
    {
        const auto leaf = static_pointer_cast<BVHLeafNode>(node);
        const tfloat hit = leaf->triangle.intersect(ray);
        ray.t = min(ray.t, hit);
    }
    else
    {
        const auto internal = static_pointer_cast<BVHInternalNode>(node);
        tfloat tmin, tmax;

        if (intersects(internal->bbox, ray, tmin, tmax))
        {
            if (tmin < ray.t)
            {
                if (ray.dir[node->type] > 0)
                {
                    recursiveTrace(ray, internal->L);
                    recursiveTrace(ray, internal->R);
                }
                else
                {
                    recursiveTrace(ray, internal->R);
                    recursiveTrace(ray, internal->L);
                }
            }
        }
    }
}

