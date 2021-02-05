#include "shapes.h"

using math::Vec2;
using math::Vec3;

Transform2d::Transform2d(Vec2 pos_, Vec2 scale_, float rotation_)
    : pos(pos_), scale(scale_), rotation(rotation_)
{}

Transform3d::Transform3d(Vec3 pos_, Vec3 scale_, float rotation_)
    : pos(pos_), scale(scale_), rotation(rotation_)
{}
