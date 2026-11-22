// Program modules next to the title (MK8 multi-module steps 2-4, port/patch_modules.py): Mario Kart 8's Turbo.rpx
// imports from mvplayer.rpl (the video player behind the replay/Miiverse screens). The module is translated on its
// own at moved addresses (CAFE_RPL_REBASE) with its tables renamed (port/module_rename.py) and compiled into the
// same program. At start this file maps the module's sections, adds its functions to the indirect-call table, gives
// its data imports their objects, binds the title's function imports to the module's exports and runs the module's
// entry (reason "load") on the title's main thread before the title's own pre-init.
// The title's DATA imports from the module are resolved when the title is translated and loaded
// (CAFE_DATA_IMPORT_MAP, data-imports.map next to the program).

#include "cafe/export.h"
#include "cafe/generated.h"
#include "cafe/layout.h"
#include "cafe/ppc_ops.h"
#include "cafe/runtime.h"
#include "cafe/sysmem.h"
#include "cafe/vfs.h"

#include "rpx.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

extern "C" const cafe::GuestFunction cafe_mod_mvplayer_functions[];
extern "C" const size_t cafe_mod_mvplayer_function_count;
extern "C" const cafe::GuestImport cafe_mod_mvplayer_imports[];
extern "C" const size_t cafe_mod_mvplayer_import_count;
extern "C" const cafe::ProgramInfo cafe_mod_mvplayer_info;

// Set by this file, called by game_main.cpp after the title is loaded (null in ports without modules).
extern void (*cafegame_module_loader)(cafe::LoadedImage& title);

namespace {

struct Module {
    const char* name;     // as the title imports it
    const char* file;     // in code/
    const char* rebase;   // CAFE_RPL_REBASE it was translated with
    const char* first_slot; // CAFE_DATA_IMPORT_FIRST it was translated with
    const cafe::GuestFunction* functions;
    const size_t* function_count;
    const cafe::GuestImport* imports;
    const size_t* import_count;
    const cafe::ProgramInfo* info;
    uint32_t entry = 0;
    uint32_t handle = 0;
    std::map<std::string, uint32_t> exports;
};

Module g_modules[] = {
    {"mvplayer", "mvplayer.rpl", "06000000,00300000", "32", cafe_mod_mvplayer_functions, &cafe_mod_mvplayer_function_count,
     cafe_mod_mvplayer_imports, &cafe_mod_mvplayer_import_count, &cafe_mod_mvplayer_info},
};

uint32_t g_title_preinit = 0;

uint32_t export_address(const char* module, const char* name) {
    for (const Module& m : g_modules) {
        if (std::strcmp(m.name, module) != 0) continue;
        const auto it = m.exports.find(name);
        if (it != m.exports.end()) return it->second;
    }
    cafe::fatal("module export %s:%s not found", module, name);
}

// The title's pre-init: first every module's entry (as the loader runs them), then the title's own.
void preinit(cafe::PPCContext& ctx, uint8_t* base) {
    const uint32_t a3 = ctx.r[3], a4 = ctx.r[4], a5 = ctx.r[5];
    for (Module& m : g_modules) {
        std::fprintf(stderr, "cafegame: module %s: entry 0x%08X\n", m.name, m.entry);
        const uint32_t result = cafe::call_guest(ctx, m.entry, {m.handle, 1u});
        std::fprintf(stderr, "cafegame: module %s: entry returned %d\n", m.name, static_cast<int32_t>(result));
    }
    if (g_title_preinit != 0) {
        ctx.r[3] = a3;
        ctx.r[4] = a4;
        ctx.r[5] = a5;
        cafe_ppc_lookup(g_title_preinit)(ctx, base);
    }
}

void load_modules(cafe::LoadedImage& title) {
    using namespace cafe;
    for (Module& m : g_modules) {
        const auto bytes = vfs::read_whole(std::string("/vol/code/") + m.file);
        if (!bytes) fatal("module %s: no code/%s", m.name, m.file);
        if (sha256_hex(*bytes) != m.info->rpx_sha256) fatal("module %s: code/%s is not the file it was translated from", m.name, m.file);
        // Loaded as cafe-recomp loaded it when translating (same moves, same data-import slots).
        const char* saved_map = std::getenv("CAFE_DATA_IMPORT_MAP");
        const std::string map = saved_map ? saved_map : "";
        ::setenv("CAFE_RPL_REBASE", m.rebase, 1);
        ::setenv("CAFE_DATA_IMPORT_FIRST", m.first_slot, 1);
        ::unsetenv("CAFE_DATA_IMPORT_MAP");
        const rpx::Image image = rpx::load(*bytes);
        ::unsetenv("CAFE_RPL_REBASE");
        ::unsetenv("CAFE_DATA_IMPORT_FIRST");
        if (saved_map) ::setenv("CAFE_DATA_IMPORT_MAP", map.c_str(), 1);
        m.entry = image.entry_point;
        uint32_t data_end = 0;
        for (const auto& section : image.sections) {
            if (section.type == 0x80000001u && !section.data.empty()) { // .fexports / .dexports, relocated
                const uint32_t count = rpx::read_be32(section.data.data());
                for (uint32_t k = 0; k < count; ++k) {
                    const uint32_t value = rpx::read_be32(section.data.data() + 8 + k * 8);
                    const uint32_t name = rpx::read_be32(section.data.data() + 12 + k * 8) & 0x7FFFFFFFu;
                    if (name < section.data.size()) m.exports[reinterpret_cast<const char*>(section.data.data() + name)] = value;
                }
                continue;
            }
            if (!section.allocated() || section.size == 0 || section.type == rpx::kShtRplImports || section.address == 0) continue;
            if (section.type != rpx::kShtProgbits && section.type != rpx::kShtNobits) continue;
            commit_guest_memory(section.address, section.size);
            if (section.type == rpx::kShtProgbits) {
                std::memcpy(guest_pointer(section.address), section.data.data(), section.size);
            } else {
                std::memset(guest_pointer(section.address), 0, section.size);
            }
            if (section.address >= layout::kMem2Base) data_end = std::max(data_end, section.address + section.size);
        }
        for (size_t i = 0; i < *m.function_count; ++i) register_guest_function(m.functions[i].address, m.functions[i].function);
        for (size_t i = 0; i < *m.import_count; ++i) {
            const GuestImport& import = m.imports[i];
            if (!import.is_data) {
                register_guest_function(import.stub_address, import.function);
            } else if (const DataExport* data = find_data_export(import.module, import.name)) {
                data->initialize(import.address);
            } else {
                std::fprintf(stderr, "cafegame: module %s: data import %s:%s is not provided\n", m.name, import.module, import.name);
            }
        }
        m.handle = system_alloc(64);
        // The title's heap starts after the module's data.
        title.data_end = std::max(title.data_end, (data_end + 0xFFFFu) & ~0xFFFFu);
        std::fprintf(stderr, "cafegame: module %s loaded: entry 0x%08X, %zu functions, %zu exports, data up to 0x%08X\n", m.name,
                     m.entry, *m.function_count, m.exports.size(), data_end);
    }
    g_title_preinit = title.preinit_user;
    title.preinit_user = register_host_function(preinit, "module entries + title pre-init");
}

const bool g_registered = (cafegame_module_loader = load_modules, true);

} // namespace

// The title's function imports from mvplayer, bound to the module's exports.
#define CAFE_BIND(module, symbol)                                                                   \
    extern "C" void imp_##module##_##symbol(cafe::PPCContext& ctx, uint8_t* base) {               \
        static const uint32_t address = export_address(#module, #symbol);                          \
        cafe_ppc_lookup(address)(ctx, base);                                                       \
    }
CAFE_BIND(mvplayer, Create__8MVPlayerSFv)
CAFE_BIND(mvplayer, Destroy__8MVPlayerSFP8MVPlayer)
CAFE_BIND(mvplayer, Draw__6MVViewFv)
CAFE_BIND(mvplayer, Shutdown__8MVPlayerSFv)
