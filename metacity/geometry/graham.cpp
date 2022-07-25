#include "graham.hpp"   

using htvec2 = glm::highp_f64vec2;

// returns -1 if a -> b -> c forms a counter-clockwise turn,
// +1 for a clockwise turn, 0 if they are collinear
int ccw(const htvec2 & a, const htvec2 & b, const htvec2 & c) {
    double area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (abs(area) < 1e-6) 
        return 0;


    if (area > 0)
        return -1;
    else //if (area < 0)
        return 1;
}
 

// returns square of Euclidean distance between two points
int sqrDist(const htvec2 & a, const htvec2 & b)  {
    int dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// used for sorting points according to polar order w.r.t the pivot
bool POLAR_ORDER(const htvec2 & a, const htvec2 & b, const htvec2 & pivot)  {
    int order = ccw(pivot, a, b);
    if (order == 0)
        return sqrDist(pivot, a) < sqrDist(pivot, b);
    return (order == -1);
}

vector<tvec2> grahamScan(vector<tvec2> & points_) 
{
    vector<htvec2> points;
    for (const auto & point : points_) {
        points.emplace_back(point);
    }
    stack<htvec2> hull;
    vector<tvec2> result;

    if (points.size() < 3)
        return result;

    // find the point having the least y coordinate (pivot),
    // ties are broken in favor of lower x coordinate
    int leastY = 0;
    for (int i = 1; i < points.size(); i++)
        if (points[i].y < points[leastY].y || (points[i].y == points[leastY].y && points[i].x < points[leastY].x))
            leastY = i;


    // swap the pivot with the first point
    htvec2 temp = points[0];
    points[0] = points[leastY];
    points[leastY] = temp;

    // sort the remaining point according to polar order about the pivot
    const htvec2 pivot = points[0];
    sort(points.begin() + 1, points.end(), [&pivot](const htvec2 & a, const htvec2 & b) {
        return POLAR_ORDER(a, b, pivot);
    });

    size_t size = points.size();
    //for(size_t i = 1; i < points.size(); i++) {
    //  //when the angle of ith and (i+1)th elements are same, remove points
    //    while(i < points.size() - 1 && (ccw(pivot, points[i], points[i + 1]) == 0))
    //        i++;
    //    points[size++] = points[i];
    //}

   if (size < 3)
       return result;
    
    hull.push(points[0]);
    hull.push(points[1]);
    hull.push(points[2]);

    htvec2 top;
    for (size_t i = 3; i < size; ++i) {
        top = hull.top();
        hull.pop();
        while (hull.size() > 1 && ccw(hull.top(), top, points[i]) != -1)   {
            top = hull.top();
            hull.pop();
        }
        hull.push(top);
        hull.push(points[i]);
    }


    while (!hull.empty()) {
        result.emplace_back(hull.top());
        hull.pop();
    }

    return result;
}