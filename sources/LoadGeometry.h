#pragma once

#include <optix.h>
#include <optixu/optixpp.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <string>
#include "types.h"


struct Mesh
{
    optix::Material material;      // optional single matl override

    optix::Program intersection;  // optional
    optix::Program bounds;        // optional

    optix::Program closest_hit;   // optional multi matl override
    optix::Program any_hit;       // optional

    // Output
    optix::GeometryInstance geom_instance;
    optix::float3 bbox_min;
    optix::float3 bbox_max;

    int num_triangles;
};


optix::Aabb createGeometry(optix::Context ctx, const std::string &filename) {
    const std::string ptx_path = ptxPath("triangle_mesh.cu");

    optix::Group group = ctx->createGroup();
    group->setAcceleration(ctx->createAcceleration("Trbvh"));

    int num_triangles = 0;
    optix::Aabb aabb;

    Mesh mesh;
    // override defaults
    mesh.intersection = context->createProgramFromPTXFile(ptx_path, "mesh_intersect_refine");
    mesh.bounds = context->createProgramFromPTXFile(ptx_path, "mesh_bounds");
    mesh.material = glass_material;

    loadMesh(filenames[i], mesh, xforms[i]);
    geometry_group->addChild(mesh.geom_instance);

    aabb.include(mesh.bbox_min, mesh.bbox_max);

    std::cerr << filenames[i] << ": " << mesh.num_triangles << std::endl;
    num_triangles += mesh.num_triangles;

    printf("Total triangle count: %d\n", num_triangles);


    {
        // Ground plane
        GeometryGroup geometry_group = context->createGeometryGroup();
        geometry_group->setAcceleration(context->createAcceleration("NoAccel"));
        top_group->addChild(geometry_group);
        const std::string floor_ptx = ptxPath("parallelogram_iterative.cu");
        GeometryInstance instance = sutil::createOptiXGroundPlane(context, floor_ptx, aabb, ground_material, 3.0f);
        geometry_group->addChild(instance);
    }

    context["top_object"]->set(top_group);

    return aabb;
}
