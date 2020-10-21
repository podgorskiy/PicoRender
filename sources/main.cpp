#include <spdlog/spdlog.h>
#include <time.h>
#include "rnd.h"
#include "sampling.h"
#include "Camera.h"
#include <chrono>

#include <optix.h>
#include <optixu/optixpp.h>

#include "main.h"
#include "ConfigReader.h"

#include <stb_image_write.h>

extern "C" const char render_program[];
extern "C" const char sphere_program[];
extern "C" const char materials_program[];


optix::Program sphereBBoxProgram;
optix::Program sphereIntersectProgram;
optix::Program lambertianProgram;

optix::Material lmat;

void SetMaterial(optix::GeometryInstance gi, vec3 albedo)
{
    gi->setMaterialCount(1);
    gi->setMaterial(/*ray type:*/0, lmat);
    gi["albedo"]->setFloat(to_cuda(albedo));
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

optix::GeometryGroup createScene(optix::Context& context)
{
    rnd::RandomState rs;
    // first, create all geometry instances (GIs), and, for now,
    // store them in a std::vector. For ease of reference, I'll
    // stick wit the 'd_list' and 'd_world' names used in the
    // reference C++ and CUDA codes.
    std::vector<optix::GeometryInstance> d_list;

    auto sphere = createSphere(context, vec3(0.f, -1000.0f, -1.f), 1000.f);
    SetMaterial(sphere, vec3(0.5f, 0.5f, 0.5f));
    d_list.push_back(sphere);

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = rs.rand();
            vec3 center(a + rs.rand(), 0.2f, b + rs.rand());
            if (choose_mat < 0.8f) {
                auto sphere = createSphere(context, center, 0.2f);
                SetMaterial(sphere, vec3(rs.rand() * rs.rand(), rs.rand() * rs.rand(), rs.rand() * rs.rand()));
                d_list.push_back(sphere);
            }
//            else if (choose_mat < 0.95f) {
//                d_list.push_back(createSphere(center, 0.2f,
//                                              Metal(vec3f(0.5f * (1.0f + rs.rand()), 0.5f * (1.0f + rs.rand()),
//                                                          0.5f * (1.0f + rs.rand())), 0.5f * rs.rand())));
//            } else {
//                d_list.push_back(createSphere(center, 0.2f, Dielectric(1.5f)));
//            }
        }
    }
    //d_list.push_back(createSphere(vec3(0.f, 1.f, 0.f), 1.f, Dielectric(1.5f)));
    //d_list.push_back(createSphere(context, vec3(-4.f, 1.f, 0.f), 1.f, Lambertian(vec3(0.4f, 0.2f, 0.1f), lmat)));
    //d_list.push_back(createSphere(vec3(4.f, 1.f, 0.f), 1.f, Metal(vec3(0.7f, 0.6f, 0.5f), 0.0f)));

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

    optix::Context ctx = optix::Context::create();
    ctx->setRayTypeCount(1);
    ctx->setStackSize(3000);
    ctx->setEntryPointCount(1);

    optix::Program renderProgram = ctx->createProgramFromPTXString(render_program, "Render");
    optix::Program missProgram = ctx->createProgramFromPTXString(render_program, "Miss");
    lambertianProgram = ctx->createProgramFromPTXString(materials_program, "lambertian_hit");
    sphereBBoxProgram = ctx->createProgramFromPTXString(sphere_program, "get_bounds");
    sphereIntersectProgram = ctx->createProgramFromPTXString(sphere_program, "hit_sphere");

    ctx->setRayGenerationProgram(0, renderProgram);
    ctx->setMissProgram(0, missProgram);

    lmat = ctx->createMaterial();
    lmat->setClosestHitProgram(0, lambertianProgram);

    optix::Buffer pixelBuffer = ctx->createBuffer(RT_BUFFER_OUTPUT);
    pixelBuffer->setFormat(RT_FORMAT_FLOAT4);
    pixelBuffer->setSize(width, height);

    optix::GeometryGroup world = createScene(ctx);
    ctx["world"]->set(world);

    ctx["pixelBuffer"]->set(pixelBuffer);
    ctx["camera_origin"]->setFloat(make_float3(0., 2., -10.));
    ctx["camera_lookat"]->setFloat(make_float3(0., 0., 0.));
    ctx["camera_up"]->setFloat(make_float3(0., 1., 0.));
    ctx["camera_vfov"]->setFloat(40.);
    ctx["camera_aperture"]->setFloat(0.2);
    ctx["camera_focusDist"]->setFloat(length(vec3(10.0, 10.0, 0.0)));

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
