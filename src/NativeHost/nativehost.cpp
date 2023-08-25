#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <limits.h>
#include <optional>
#include <iostream>
#include "./inc/nethost.hpp"
#include "./inc/hostfxr.hpp"
#include "./inc/delegates.hpp"
#define TEXT(s) s
#define DIR '/'
#define MAX_PATH PATH_MAX

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using handle_t = void*;
using string_t = std::basic_string<char_t>;

struct HostFXRDLL {
    HostFXR::close close;
    HostFXR::getDelegate getDelegate;
    HostFXR::initRuntimeConfig init;
};

// ============================================================================

/**
 * @brief Loads a library from the given path
 *
 * @param path The path to the library
 * @return The handle to the library
 */
static handle_t openDLL(const char_t* path) {
	handle_t handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
	return !handle ? throw dlerror() : handle;
}

/**
 * @brief Gets the address of the given export from the given library
 *
 * @param handle The handle to the library
 * @param name The name of the export
 * @return The address of the export
 */
template <typename T>
static T fetchSymbol(void* handle, const char_t* name) {
	void* symbol = dlsym(handle, name);
	return !symbol ? throw dlerror() : (T)(symbol);
}

/**
 * @brief Gets the root path of the given path
 * 
 * @param path The path to get the root path of
 * @return The root path of the given path or nullopt on failure
 */
static std::optional<string_t> getRootPath(const char_t* path) {
    char_t pathBuffer[MAX_PATH];
    if (realpath(path, pathBuffer) == nullptr) {
        std::cerr << "getRootPath failed: " << strerror(errno) << std::endl;
        return std::nullopt;
    }

    string_t rootPath(pathBuffer);
    size_t lastSlash = rootPath.find_last_of(DIR);
    if (lastSlash == string_t::npos) {
        std::cerr << "getRootPath failed: find_last_of" << std::endl;
        return std::nullopt;
    }
    return rootPath.substr(0, lastSlash + 1);
}

/**
 * @brief Loads the hostfxr library
 * @return The hostfxr library symbol table
 */
static std::optional<HostFXRDLL> loadHostFXR(void) {
    char_t buffer[MAX_PATH];
    size_t buffsize = sizeof(buffer) / sizeof(char_t);

    int code = get_hostfxr_path(buffer, &buffsize, nullptr);
    if (code != 0) {
        std::cerr << "get_hostfxr_path failed" << std::endl;
        return std::nullopt;
    }

    void* lib = openDLL(buffer);
    return HostFXRDLL {
        fetchSymbol<HostFXR::close>(lib, TEXT("hostfxr_close")),
        fetchSymbol<HostFXR::getDelegate>(lib, TEXT("hostfxr_get_runtime_delegate")),
        fetchSymbol<HostFXR::initRuntimeConfig>(lib, TEXT("hostfxr_initialize_for_runtime_config"))
    };
}

/**
 * @brief Loads .NET Core
 * 
 * @param configPath The path to the runtime config
 * @param fxr The hostfxr library symbol table
 * @return The load assembly function pointer or nullopt on failure
 */
static std::optional<MCS::Delegates::load> loadDotnetCore(const char_t *configPath, const HostFXRDLL& fxr) {
    void* assembly = nullptr;
    HostFXR::Handle context = nullptr;

    int32_t code = fxr.init(configPath, nullptr, &context);
    if (code != 0 || context == nullptr) {
        std::cerr << "Init failed: " << std::hex << std::showbase << code << std::endl;
        fxr.close(context);
        return std::nullopt;
    }

    code = fxr.getDelegate(context, HostFXR::DelegateType::LoadAssmAndGetFnPtr, &assembly);
    if (code != 0 || assembly == nullptr) {
        std::cerr << "Get delegate failed: " << std::hex << std::showbase << code << std::endl;
        fxr.close(context);
        return std::nullopt;
    }

    fxr.close(context);
    return (MCS::Delegates::load)(assembly);
}

// ============================================================================

i32 main(i32 argc, const char_t* argv[])
{
    const string_t rootPath = getRootPath(argv[0]).value();
    const HostFXRDLL fxrDLL = loadHostFXR().value();

    const string_t configPath = rootPath + TEXT("DotNetLib.runtimeconfig.json");
    const auto netCoreAssembly = loadDotnetCore(configPath.c_str(), fxrDLL).value();

	const string_t dotnetlibPath = rootPath + TEXT("DotNetLib.dll");
	const char_t *dotnetType = TEXT("DotNetLib.Lib, DotNetLib");
	const char_t *dotnetTypeMethod = TEXT("Hello");

    MCS::Delegates::componentEntryPoint hello = nullptr;
    i32 code = netCoreAssembly(
        dotnetlibPath.c_str(),  // assemblyPath
        dotnetType,             // typeName
        dotnetTypeMethod,       // methodName
        nullptr,                // delegateTypeName
        nullptr,                // reserved
        (void**)&hello          // delegate
    );

    if (code != 0 || hello == nullptr) {
        std::cerr << "loadAssembly 1 failed: " << std::hex << std::showbase << code << std::endl;
        return EXIT_FAILURE;
    }

    struct libArgs {
        const char_t* message;
        i32 number;
    };

    for (i32 i = 0; i < 3; ++i) {
        libArgs args { TEXT("from host!"), i };
        hello(&args, sizeof(args));
    }

    typedef void (DELEGATE_CALLTYPE *customEntryPointFn)(libArgs args);
    customEntryPointFn custom = nullptr;
    code = netCoreAssembly(
        dotnetlibPath.c_str(),  // assemblyPath
        dotnetType,             // typeName
        TEXT("CustomEntryPoint"),   // methodName
        TEXT("DotNetLib.Lib+CustomEntryPointDelegate, DotNetLib"), // delegateTypeName
        nullptr,                // reserved
        (void**)&custom         // delegate
    );

    if (code != 0 || custom == nullptr) {
        std::cerr << "loadAssembly failed: " << std::hex << std::showbase << code << std::endl;
        return EXIT_FAILURE;
    }

    custom({ TEXT("from host!"), -1 });
    return EXIT_SUCCESS;
}
