#pragma once
#include "models.hpp"


class MultiTimePoint : public BaseModel {
public: 
    void set_points_from_b64(const string & data);
    void set_start_time(const uint32_t & start_time);

    const uint32_t get_start_time() const;
    const uint32_t size() const;

    inline const tvec3 & operator[] (const size_t t) const {
        return points[t];
    };


    virtual json serialize() const override;
    virtual void deserialize(const json data) override;

    virtual const char * type() const override;
    virtual shared_ptr<Model> transform() const override;
    
protected:
    uint32_t start;
    vector<tvec3> points;
};



