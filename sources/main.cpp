#include <spdlog/spdlog.h>
#include <time.h>
#include "rnd.h"
#include "sampling.h"
#include "MaterialFactory.h"
#include <chrono>

#include <optix.h>
#include <optixu/optixpp.h>
#include <tiny_obj_loader.h>

#include "main.h"
#include "ConfigReader.h"

#include <stb_image_write.h>

extern "C" const char render_program[];
extern "C" const char sphere_program[];
extern "C" const char mesh_program[];


optix::Program sphereBBoxProgram;
optix::Program sphereIntersectProgram;

void SetMaterial(optix::GeometryInstance gi, optix::Material mat, vec3 albedo)
{
    gi->setMaterialCount(1);
    gi->setMaterial(/*ray type:*/0, mat);
    gi["albedo"]->setFloat(albedo);
}

optix::GeometryInstance createSphere(optix::Context& context, const vec3 &center, const float radius)
{
    optix::Geometry geometry = context->createGeometry();
    geometry->setPrimitiveCount(1);
    geometry->setBoundingBoxProgram(sphereBBoxProgram);
    geometry->setIntersectionProgram(sphereIntersectProgram);
    geometry["center"]->setFloat(center.x, center.y, center.z);
    geometry["radius"]->setFloat(radius);
    optix::GeometryInstance gi = context->createGeometryInstance();
    gi->setGeometry(geometry);
    return gi;
}

optix::GeometryGroup createScene(optix::Context& context, const MatFactory& matFactory)
{
    rnd::RandomState rs;
    std::vector<optix::GeometryInstance> d_list;

    auto lambert = matFactory(Lambertian);

    auto sphere = createSphere(context, make_float3(0.f, -1000.0f, -1.f), 1000.f);
    SetMaterial(sphere, lambert, make_float3(0.5f, 0.5f, 0.5f));

    d_list.push_back(sphere);

    // now, create the optix world that contains all these GIs
    optix::GeometryGroup d_world = context->createGeometryGroup();
    d_world->setAcceleration(context->createAcceleration("Bvh"));
    d_world->setChildCount((int) d_list.size());
    for (int i = 0; i < d_list.size(); i++)
        d_world->setChild(i, d_list[i]);

    // that all we have to do, the rest is up to optix
    return d_world;
}


int main(int argc, char* argv[])
{
    Config *config = new Config;
    config->LoadConfig("config.txt");
    int width = config->GetField("width")->GetInt();
    int height = config->GetField("height")->GetInt();
    const char *outfile_prefix = config->GetField("outfile_prefix")->GetStr();
    const char *obj_file = config->GetField("obj")->GetStr();

    optix::Context ctx = optix::Context::create();
    ctx->setRayTypeCount(1);
    ctx->setStackSize(3000);
    ctx->setEntryPointCount(1);

    optix::Program meshBoundsProgram = ctx->createProgramFromPTXString(mesh_program, "mesh_bounds");
    optix::Program meshIntersectProgram = ctx->createProgramFromPTXString(mesh_program, "mesh_intersect");

    optix::Group root = ctx->createGroup();
    root->setAcceleration( ctx->createAcceleration( "Trbvh" ) );

    auto matFactory = MakeMaterialFactory(ctx);

    tinyobj::ObjReader obj;
    tinyobj::ObjReaderConfig cfg;
    cfg.vertex_color = false;
    obj.ParseFromFile(obj_file, cfg);

    std::vector<optix::Material> optix_materials;
    auto mat = matFactory(Lambertian);
    mat["albedo"]->setFloat(make_float3(1.0, 0.2, 0.2));

    optix_materials.push_back(mat);

    optix::GeometryGroup group = ctx->createGeometryGroup();
    group->setAcceleration(ctx->createAcceleration("Trbvh"));

    int num_positions = obj.GetAttrib().vertices.size() / 3;
    int num_normals = obj.GetAttrib().normals.size() / 3;
    int num_texcoords = obj.GetAttrib().texcoords.size() / 2;

    auto positions = ctx->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_positions);
    auto normals = ctx->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_normals);
    auto texcoords = ctx->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, num_texcoords);

    {
        auto p_positions = reinterpret_cast<float *>  ( positions->map());
        auto p_normals = reinterpret_cast<float *>  ( num_normals > 0 ? normals->map() : 0 );
        auto p_texcoords = reinterpret_cast<float *>  ( num_texcoords > 0 ? texcoords->map() : 0 );

        memcpy(p_positions, obj.GetAttrib().vertices.data(), sizeof(float) * num_positions * 3);
        if (num_texcoords > 0)
            memcpy(p_texcoords, obj.GetAttrib().texcoords.data(), sizeof(float) * num_texcoords * 2);
        if (num_normals > 0)
            memcpy(p_normals, obj.GetAttrib().normals.data(), sizeof(float) * num_normals * 3);

        positions->unmap();
        if (num_texcoords > 0)
            texcoords->unmap();
        if (num_normals > 0)
            normals->unmap();
    }
    for (auto& shape: obj.GetShapes())
    {
        int num_triangles = shape.mesh.indices.size() / 3;

        auto tri_indices = ctx->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, num_triangles * 3);
        auto mat_indices = ctx->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT, num_triangles);

        auto p_tri_indices = reinterpret_cast<int32_t *>( tri_indices->map());
        auto p_mat_indices = reinterpret_cast<int32_t *>( mat_indices->map());

        auto mat_ids = shape.mesh.material_ids;

        for (int i = 0; i < num_triangles; ++i)
            mat_ids[i] = 0;

        memcpy(p_tri_indices, shape.mesh.indices.data(), sizeof(tinyobj::index_t) * num_triangles * 3);
        memcpy(p_mat_indices, mat_ids.data(), sizeof(int) * num_triangles);

        tri_indices->unmap();
        mat_indices->unmap();

        optix::Geometry geometry = ctx->createGeometry();
        geometry["vertex_buffer"]->setBuffer(positions);
        geometry["normal_buffer"]->setBuffer(normals);
        geometry["texcoord_buffer"]->setBuffer(texcoords);
        geometry["material_buffer"]->setBuffer(mat_indices);
        geometry["index_buffer"]->setBuffer(tri_indices);
        geometry->setPrimitiveCount(num_triangles);
        geometry->setBoundingBoxProgram(meshBoundsProgram);
        geometry->setIntersectionProgram(meshIntersectProgram);
        auto geom_instance = ctx->createGeometryInstance(
                geometry,
                optix_materials.begin(),
                optix_materials.end()
        );
        group->addChild(geom_instance);
    }

    root->addChild(group);

    optix::Program renderProgram = ctx->createProgramFromPTXString(render_program, "Render");
    optix::Program missProgram = ctx->createProgramFromPTXString(render_program, "Miss");
    sphereBBoxProgram = ctx->createProgramFromPTXString(sphere_program, "get_bounds");
    sphereIntersectProgram = ctx->createProgramFromPTXString(sphere_program, "hit_sphere");

    ctx->setRayGenerationProgram(0, renderProgram);
    ctx->setMissProgram(0, missProgram);

    optix::Buffer pixelBuffer = ctx->createBuffer(RT_BUFFER_OUTPUT);
    pixelBuffer->setFormat(RT_FORMAT_FLOAT4);
    pixelBuffer->setSize(width, height);

    optix::GeometryGroup world = createScene(ctx, matFactory);
    root->addChild(world);

    ctx["root"]->set(root);

    ctx["pixelBuffer"]->set(pixelBuffer);
    ctx["camera_origin"]->setFloat(make_float3(0., 50., -100.));
    ctx["camera_lookat"]->setFloat(make_float3(0., 0., 0.));
    ctx["camera_up"]->setFloat(make_float3(0., 1., 0.));
    ctx["camera_vfov"]->setFloat(config->GetField("camera_vfov")->GetFloat());
    ctx["camera_aperture"]->setFloat(config->GetField("camera_aperture")->GetFloat());
    ctx["camera_focusDist"]->setFloat(length(make_float3(0.0, 50.0, 100.0)));

    int numSamples = config->GetField("sampleCount")->GetInt();
    ctx["numSamples"]->setInt(numSamples);
    int bounces = config->GetField("bounces")->GetInt();
    ctx["bounces"]->setInt(bounces);

    ctx->validate();

    spdlog::info("launching raytracing");
    auto t0 = std::chrono::system_clock::now();
    ctx->launch(0, width, height);
    auto t1 = std::chrono::system_clock::now();
    spdlog::info("done rendering, which took {:.4} seconds (for {} paths per pixel and {} bounces)",
                 std::chrono::duration<double>(t1 - t0).count(), numSamples, bounces);

    auto *pixels = (const float *) pixelBuffer->map();
    uint8_t *data8b = TOUINT8(pixels, 4 * width * height);
    std::string name("_out.png");
    stbi_write_png((outfile_prefix + name).c_str(), width, height, 4, data8b, 0);
    pixelBuffer->unmap();
}
