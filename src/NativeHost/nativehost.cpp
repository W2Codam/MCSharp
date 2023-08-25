
//// Licensed to the .NET Foundation under one or more agreements.
//// The .NET Foundation licenses this file to you under the MIT license.
//// See the LICENSE file in the project root for more information.

//// Standard headers
//#include <stdio.h>
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
//#include <iostream>
//#include "./inc/nethost.hpp"
//#include "./inc/delegates.hpp"
//#include "./inc/hostfxr.hpp"

//#ifdef WINDOWS
//#include <Windows.h>

//#define STR(s) L##s
//#define CH(c) L##c
//#define DIR_SEPARATOR L'\\'

//#else
//#include <dlfcn.h>
//#include <limits.h>

//#define STR(s) s
//#define CH(c) c
//#define DIR_SEPARATOR '/'
//#define MAX_PATH PATH_MAX

//#endif

//using string_t = std::basic_string<char_t>;

//namespace
//{
//	// Globals to hold hostfxr exports
//	HostFXR::initRuntimeConfig init_fptr;
//	HostFXR::getDelegate get_delegate_fptr;
//	HostFXR::close close_fptr;

//	// Forward declarations
//	bool load_hostfxr();
//	MCS::Delegates::load get_dotnet_load_assembly(const char_t *assembly);
//}

//#if defined(WINDOWS)
//int __cdecl wmain(int argc, wchar_t *argv[])
//#else
//int main(int argc, char *argv[])
//#endif
//{
//	// Get the current executable's directory
//	// This sample assumes the managed assembly to load and its runtime configuration file are next to the host
//	char_t host_path[MAX_PATH];
//#if WINDOWS
//	auto size = ::GetFullPathNameW(argv[0], sizeof(host_path) / sizeof(char_t), host_path, nullptr);
//	assert(size != 0);
//#else
//	auto resolved = realpath(argv[0], host_path);
//	assert(resolved != nullptr);
//#endif

//	string_t root_path = host_path;
//	auto pos = root_path.find_last_of(DIR_SEPARATOR);
//	assert(pos != string_t::npos);
//	root_path = root_path.substr(0, pos + 1);

//	//
//	// STEP 1: Load HostFxr and get exported hosting functions
//	//
//	if (!load_hostfxr())
//	{
//		assert(false && "Failure: load_hostfxr()");
//		return EXIT_FAILURE;
//	}

//	//
//	// STEP 2: Initialize and start the .NET Core runtime
//	//
//	const string_t config_path = root_path + STR("DotNetLib.runtimeconfig.json");
//	MCS::Delegates::load load_assembly_and_get_function_pointer = nullptr;
//	load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
//	assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

//	//
//	// STEP 3: Load managed assembly and get function pointer to a managed method
//	//
//	const string_t dotnetlib_path = root_path + STR("DotNetLib.dll");
//	const char_t *dotnet_type = STR("DotNetLib.Lib, DotNetLib");
//	const char_t *dotnet_type_method = STR("Hello");
//	// Function pointer to managed delegate
//	MCS::Delegates::component_entry_point_fn hello = nullptr;
//	int rc = load_assembly_and_get_function_pointer(
//			dotnetlib_path.c_str(),
//			dotnet_type,
//			dotnet_type_method,
//			nullptr /*delegate_type_name*/,
//			nullptr,
//			(void **)&hello);
//	assert(rc == 0 && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");

//	//
//	// STEP 4: Run managed code
//	//
//	struct lib_args
//	{
//		const char_t *message;
//		int number;
//	};
//	for (int i = 0; i < 3; ++i)
//	{
//		// <SnippetCallManaged>
//		lib_args args{
//				STR("from host!"),
//				i};

//		hello(&args, sizeof(args));
//	}

//	// Function pointer to managed delegate with non-default signature
//	typedef void(DELEGATE_CALLTYPE * custom_entry_point_fn)(lib_args args);
//	custom_entry_point_fn custom = nullptr;
//	rc = load_assembly_and_get_function_pointer(
//			dotnetlib_path.c_str(),
//			dotnet_type,
//			STR("CustomEntryPoint") /*method_name*/,
//			STR("DotNetLib.Lib+CustomEntryPointDelegate, DotNetLib") /*delegate_type_name*/,
//			nullptr,
//			(void **)&custom);
//	assert(rc == 0 && custom != nullptr && "Failure: load_assembly_and_get_function_pointer()");

//	lib_args args{
//			STR("from host!"),
//			-1};
//	custom(args);

//	return EXIT_SUCCESS;
//}

///********************************************************************************************
// * Function used to load and activate .NET Core
// ********************************************************************************************/

//namespace
//{
//	// Forward declarations
//	void *load_library(const char_t *);
//	void *get_export(void *, const char *);

//#ifdef WINDOWS
//	void *load_library(const char_t *path)
//	{
//		HMODULE h = ::LoadLibraryW(path);
//		assert(h != nullptr);
//		return (void *)h;
//	}
//	void *get_export(void *h, const char *name)
//	{
//		void *f = ::GetProcAddress((HMODULE)h, name);
//		assert(f != nullptr);
//		return f;
//	}
//#else
//	void *load_library(const char_t *path)
//	{
//		void *h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
//		assert(h != nullptr);
//		return h;
//	}
//	void *get_export(void *h, const char *name)
//	{
//		void *f = dlsym(h, name);
//		assert(f != nullptr);
//		return f;
//	}
//#endif

//	// Using the nethost library, discover the location of hostfxr and get exports
//	bool load_hostfxr()
//	{
//		// Pre-allocate a large buffer for the path to hostfxr
//		char_t buffer[MAX_PATH];
//		size_t buffer_size = sizeof(buffer) / sizeof(char_t);
//		int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
//		if (rc != 0)
//			return false;

//		// Load hostfxr and get desired exports
//		void *lib = load_library(buffer);
//		init_fptr = (initRuntimeConfig)get_export(lib, "hostfxr_initialize_for_runtime_config");
//		get_delegate_fptr = (getDelegate)get_export(lib, "hostfxr_get_runtime_delegate");
//		close_fptr = (close)get_export(lib, "hostfxr_close");

//		return (init_fptr && get_delegate_fptr && close_fptr);
//	}

//	// Load and initialize .NET Core and get desired function pointer for scenario
//	MCS::Delegates::load get_dotnet_load_assembly(const char_t *config_path)
//	{
//		// Load .NET Core
//		void *load_assembly_and_get_function_pointer = nullptr;
//		hostfxr_handle cxt = nullptr;
//		int rc = init_fptr(config_path, nullptr, &cxt);
//		if (rc != 0 || cxt == nullptr)
//		{
//			std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
//			close_fptr(cxt);
//			return nullptr;
//		}

//		// Get the load assembly function pointer
//		rc = get_delegate_fptr(
//				cxt,
//				HostFXRDelegateType::LoadAssmAndGetFnPtr,
//				&load_assembly_and_get_function_pointer);
//		if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
//			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

//		close_fptr(cxt);
//		return (MCS::Delegates::load)load_assembly_and_get_function_pointer;
//	}
//}

//// ============================================================================

//#include <assert.h>
//#include <iostream>
//#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <optional>
//#include "./inc/delegates.hpp"
//#include "./inc/hostfxr.hpp"
//#include "./inc/nethost.hpp"
//#define as static_cast
//#ifdef WINDOWS
//#include <Windows.h>
//#define TEXT(s) L##s
//#define DIR L'\\'
//#define main __cdecl wmain
//#else
//#include <dlfcn.h>
//#include <limits.h>
//#define TEXT(s) s
//#define DIR '/'
//#define MAX_PATH PATH_MAX
//#define main main
//#endif

//using string_t = std::basic_string<char_t>;

//// Symbols for the hostfxr library.
//struct HostFXRDLL {
//    HostFXR::close close;
//    HostFXR::getDelegate getDelegate;
//    HostFXR::initRuntimeConfig init;
//};

//// DLL Functions
//// ============================================================================

///**
// * @brief Loads a library from the given path
// *
// * @param path The path to the library
// * @return The handle to the library
// */
//static void* loadDLL(const char_t* path) {
//	void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
//	return !handle ? throw dlerror() : handle;
//}

///**
// * @brief Gets the address of the given export from the given library
// *
// * @param handle The handle to the library
// * @param name The name of the export
// * @return The address of the export
// */
//template <typename T = void*>
//static T getSymbol(void* handle, const char_t* name) {
//	void* symbol = dlsym(handle, name);
//	return !symbol ? throw dlerror() : as<T>(symbol);
//}

//// ============================================================================

///**
// * @brief Loads the hostfxr library
// * @return The hostfxr library symbol table
// */
//static std::optional<HostFXRDLL> loadHostFXRDLL(void) {
//	char_t buffer[MAX_PATH];
//	size_t buffsize = sizeof(buffer) / sizeof(char_t);

//	int code = get_hostfxr_path(buffer, &buffsize, nullptr);
//    if (code != 0) {
//        std::cerr << "get_hostfxr_path failed" << std::endl;
//        return std::nullopt;
//    }

//    void* lib = loadDLL(buffer);
//    return HostFXRDLL {
//        getSymbol<HostFXR::close>(lib, TEXT("hostfxr_close")),
//        getSymbol<HostFXR::getDelegate>(lib, TEXT("hostfxr_get_runtime_delegate")),
//        getSymbol<HostFXR::initRuntimeConfig>(lib, TEXT("hostfxr_initialize_for_runtime_config"))
//    };
//}

///** Load .NET Core */
//MCS::Delegates::load loadDotnetCore(const char_t *configPath, HostFXRDLL& fxr)
//{
//	void* assembly = nullptr;
//	HostFXR::Handle context = nullptr;

//	int32_t code = fxr.init(configPath, nullptr, &context);
//	if (code != 0 || context == nullptr)
//	{
//		std::cerr << "Init failed: " << std::hex << std::showbase << code << std::endl;
//		fxr.close(context);
//		return nullptr;
//	}

//    code = fxr.getDelegate(context, HostFXR::DelegateType::LoadAssmAndGetFnPtr, &assembly);
//	if (code != 0 || assembly == nullptr)
//		std::cerr << "Get delegate failed: " << std::hex << std::showbase << code << std::endl;
//	fxr.close(context);
//    return as<MCS::Delegates::load>(assembly);
//}

//// ============================================================================

//int main(int argc, char_t* argv[]) {
//	char_t hostPath[MAX_PATH];
//	char_t* resolved = realpath(argv[0], hostPath);
//	assert(resolved != nullptr);

//	string_t rootPath(hostPath);
//	size_t lastSlash = rootPath.find_last_of(DIR);
//	assert(lastSlash != string_t::npos);
//	rootPath = rootPath.substr(0, lastSlash);

//    auto fxrSymbols = loadHostFXRDLL();
//	if (!fxrSymbols.has_value()) {
//		std::cerr << "load_hostfxr failed" << std::endl;
//		return EXIT_FAILURE;
//	}

//	const string_t config_path = root_path + TEXT("DotNetLib.runtimeconfig.json");
//	MCS::Delegates::load assmebly(get_dotnet_load_assembly(config_path.c_str()));
//}

// ============================================================================

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
    return rootPath.substr(0, lastSlash);
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

    const string_t configPath = rootPath + DIR + TEXT("DotNetLib.runtimeconfig.json");
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
	assert(code == 0 && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");

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
