#pragma once
#include "types.hpp"

struct BBox
{
    tvec3 min;
    tvec3 max;

    inline tvec3 centroid() const
    {
        return (min + max) * tvec3(0.5);
    }
};

void for_triangle(const tvec3 t[3], BBox & b);
void for_line(const tvec3 t[2], BBox & b);
void for_bboxes(const BBox &b1, const BBox &b2, BBox & outb);
bool overlaps(const BBox &b1, const BBox &b2);
bool inside(const BBox &b, const tvec3 &p);
void set_empty(BBox &box);
BBox empty_bbox();
void extend(BBox &b1, const BBox &b2);
void extend(BBox &b1, const tvec3 &p);
tfloat midpoint(const BBox &b1, uint8_t axis);
