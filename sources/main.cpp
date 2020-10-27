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
extern "C" const char render_program_uv[];
extern "C" const char mesh_program[];
extern "C" const char mesh_program_uv[];


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

    auto matFactory = MakeMaterialFactory(ctx);

    optix::Program meshBoundsProgram = ctx->createProgramFromPTXString(mesh_program, "mesh_bounds");
    optix::Program meshBoundsProgramUV = ctx->createProgramFromPTXString(mesh_program_uv, "mesh_bounds_uv");
    optix::Program meshIntersectProgram = ctx->createProgramFromPTXString(mesh_program, "mesh_intersect");
    optix::Program meshIntersectProgramUV = ctx->createProgramFromPTXString(mesh_program_uv, "mesh_intersect_uv");

    optix::Group root = ctx->createGroup();
    optix::Group root_uv = ctx->createGroup();

    root->setAcceleration( ctx->createAcceleration( "Trbvh" ) );
    root_uv->setAcceleration( ctx->createAcceleration( "Trbvh" ) );

    tinyobj::ObjReader obj;
    tinyobj::ObjReaderConfig cfg;
    cfg.vertex_color = false;
    obj.ParseFromFile(obj_file, cfg);

    std::vector<optix::Material> optix_materials;
    auto mat = matFactory(Lambertian);
    mat["albedo"]->setFloat(make_float3(0.8, 0.2, 0.2));

    optix_materials.push_back(mat);

    optix::GeometryGroup group = ctx->createGeometryGroup();
    optix::GeometryGroup group_uv = ctx->createGeometryGroup();
    group->setAcceleration(ctx->createAcceleration("Trbvh"));
    group_uv->setAcceleration(ctx->createAcceleration("Trbvh"));

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

        optix::Geometry geometry_uv = ctx->createGeometry();
        geometry_uv["vertex_buffer"]->setBuffer(positions);
        geometry_uv["normal_buffer"]->setBuffer(normals);
        geometry_uv["texcoord_buffer"]->setBuffer(texcoords);
        geometry_uv["material_buffer"]->setBuffer(mat_indices);
        geometry_uv["index_buffer"]->setBuffer(tri_indices);
        geometry_uv->setPrimitiveCount(num_triangles);
        geometry_uv->setBoundingBoxProgram(meshBoundsProgramUV);
        geometry_uv->setIntersectionProgram(meshIntersectProgramUV);
        auto geom_instance_uv = ctx->createGeometryInstance(
                geometry_uv,
                optix_materials.begin(),
                optix_materials.end()
        );
        group_uv->addChild(geom_instance_uv);
    }

    root->addChild(group);
    root_uv->addChild(group_uv);

    //optix::Program renderProgram = ctx->createProgramFromPTXString(render_program, "Render");
    optix::Program renderProgram = ctx->createProgramFromPTXString(render_program_uv, "RenderUV");
    optix::Program missProgram = ctx->createProgramFromPTXString(render_program, "Miss");

    ctx->setRayGenerationProgram(0, renderProgram);
    ctx->setMissProgram(0, missProgram);

    optix::Buffer pixelBuffer = ctx->createBuffer(RT_BUFFER_OUTPUT);
    pixelBuffer->setFormat(RT_FORMAT_FLOAT4);
    pixelBuffer->setSize(width, height);

    ctx["root"]->set(root);
    ctx["root_uv"]->set(root_uv);

    ctx["pixelBuffer"]->set(pixelBuffer);
    ctx["camera_origin"]->setFloat(make_float3(0., 0.5, -1.));
    ctx["camera_lookat"]->setFloat(make_float3(0., 0., 0.));
    ctx["camera_up"]->setFloat(make_float3(0., 1., 0.));
    ctx["camera_vfov"]->setFloat(config->GetField("camera_vfov")->GetFloat());
    ctx["camera_aperture"]->setFloat(config->GetField("camera_aperture")->GetFloat());
    ctx["camera_focusDist"]->setFloat(length(make_float3(0.0, .5, 1.0)));

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
