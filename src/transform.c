////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2017 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#include "transform.h"

void
shiz_transform_project_ortho(mat4x4 world,
                             mat4x4 model,
                             SHIZViewport const viewport)
{
    mat4x4 view;
    mat4x4_identity(view);

    mat4x4 projection;
    mat4x4_identity(projection);
    
    mat4x4_ortho(projection,
                 0, viewport.screen.width,
                 0, viewport.screen.height,
                 -1 /* near */, 1 /* far */);

    mat4x4 model_view;
    mat4x4_mul(model_view, model, view);
    mat4x4_mul(world, projection, model_view);
}

void
shiz_transform_translate_rotate_scale(mat4x4 model,
                                      SHIZVector3 const translation,
                                      float const angle,
                                      float const scale)
{
    mat4x4 translated;
    mat4x4_translate(translated, translation.x, translation.y, translation.z);

    mat4x4 rotated;
    mat4x4_identity(rotated);
    mat4x4_rotate_Z(rotated, rotated, angle);

    mat4x4 scaled;
    mat4x4_identity(scaled);
    mat4x4_scale_aniso(scaled, scaled, scale, scale, scale);

    mat4x4_mul(model, translated, rotated);
    mat4x4_mul(model, model, scaled);
}
