#include "MaterialFactory.h"
#include <string>
#include <map>
#include <assert.h>

extern "C" const char materials_program[];


MatFactory MakeMaterialFactory(optix::Context& ctx)
{
    std::map<MaterialTypes, optix::Program> material_programs;

    std::vector<std::tuple<MaterialTypes, std::string, const char*> > material_sources = {
            { Lambertian, "lambertian_hit", materials_program },
    };

    for (const auto& m: material_sources)
    {
        MaterialTypes type;
        std::string entry_point;
        const char* ptx_source;
        std::tie(type, entry_point, ptx_source) = m;
        auto lambertianProgram = ctx->createProgramFromPTXString(ptx_source, entry_point);
        material_programs[type] = lambertianProgram;
    }

    return [material_programs, ctx](MaterialTypes type)
    {
        optix::Context context = ctx;
        optix::Material mat = context->createMaterial();
        auto it = material_programs.find(type);
        assert(it != material_programs.end());
        mat->setClosestHitProgram(0, it->second);
        return mat;
    };
}
