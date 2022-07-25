#include "bbox.hpp"

using namespace std;

void for_triangle(const tvec3 t[3], BBox & box)
{
    box.min.x = min(t[0].x, min(t[1].x, t[2].x));
    box.min.y = min(t[0].y, min(t[1].y, t[2].y));
    box.min.z = min(t[0].z, min(t[1].z, t[2].z));
    box.max.x = max(t[0].x, max(t[1].x, t[2].x));
    box.max.y = max(t[0].y, max(t[1].y, t[2].y));
    box.max.z = max(t[0].z, max(t[1].z, t[2].z));
}

void for_line(const tvec3 l[2], BBox & box)
{
    box.min.x = min(l[0].x, l[1].x);
    box.min.y = min(l[0].y, l[1].y);
    box.min.z = min(l[0].z, l[1].z);
    box.max.x = max(l[0].x, l[1].x);
    box.max.y = max(l[0].y, l[1].y);
    box.max.z = max(l[0].z, l[1].z);
}

void for_bboxes(const BBox &b1, const BBox &b2, BBox & outb)
{
    outb.min.x = min(b1.min.x, b2.min.x);
    outb.min.y = min(b1.min.y, b2.min.y);
    outb.min.z = min(b1.min.z, b2.min.z);
    outb.max.x = max(b1.max.x, b2.max.x);
    outb.max.y = max(b1.max.y, b2.max.y);
    outb.max.z = max(b1.max.z, b2.max.z);
}

bool overlaps(const BBox &b1, const BBox &b2)
{
    // If one rectangle is on left side of other               or above
    return !((b1.min.x >= b2.max.x || b2.min.x >= b1.max.x) || (b1.min.y >= b2.max.y || b2.min.y >= b1.max.y));
}

bool inside(const BBox &b, const tvec3 &p)
{
    return ((b.min.x <= p.x) && (p.x <= b.max.x)) && ((b.min.y <= p.y) && (p.y <= b.max.y));
}

void set_empty(BBox &box)
{
    box.min = tvec3(FLT_MAX);
    box.max = tvec3(-FLT_MAX);
}

BBox empty_bbox()
{
    BBox box;
    box.min = tvec3(FLT_MAX);
    box.max = tvec3(-FLT_MAX);
    return box;
}

void extend(BBox &b1, const BBox &b2)
{
    b1.min.x = min(b1.min.x, b2.min.x);
    b1.min.y = min(b1.min.y, b2.min.y);
    b1.min.z = min(b1.min.z, b2.min.z);
    b1.max.x = max(b1.max.x, b2.max.x);
    b1.max.y = max(b1.max.y, b2.max.y);
    b1.max.z = max(b1.max.z, b2.max.z);
}

void extend(BBox &b1, const tvec3 &p)
{
    b1.min.x = min(b1.min.x, p.x);
    b1.min.y = min(b1.min.y, p.y);
    b1.min.z = min(b1.min.z, p.z);
    b1.max.x = max(b1.max.x, p.x);
    b1.max.y = max(b1.max.y, p.y);
    b1.max.z = max(b1.max.z, p.z);
}

tfloat midpoint(const BBox &b1, uint8_t axis)
{
    return (b1.min[axis] + b1.max[axis]) / 2;
}
