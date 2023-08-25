// ============================================================================
// Copyright (C) 2023 - W2Wizard & TechDaan
// See the LICENSE file in the project root for more information.
// ============================================================================
// This file is a modified version of the original file from the .NET Core
// Runtime repository. The original file is licensed under the MIT license.
// See: https://github.com/dotnet/runtime/blob/main/src/native/corehost
// ============================================================================

#pragma once

#include <stdint.h>

// Types, Defines, ...
// ============================================================================

#if defined(_WIN32)
	#define DELEGATE_CALLTYPE __stdcall
	#ifdef _WCHAR_T_DEFINED
		typedef wchar_t char_t;
	#else
		typedef unsigned short char_t;
	#endif
#else
	#define DELEGATE_CALLTYPE
	typedef char char_t;
#endif

#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)

// Functions
// ============================================================================

namespace MCS::Delegates
{
	
/**
 * @brief Signature of delegate returned by 
 * coreclr_delegate_type::load_assembly_and_get_function_pointer
 * 
 * @param[in] assemblyPath Fully qualified path to assembly
 * @param[in] typeName Assembly qualified type name
 * @param[in] methodName Public static method name compatible with delegateType
 * @param[in] delegateTypeName Assembly qualified delegate type name or null,
 * or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
 * the UnmanagedCallersOnlyAttribute.
 * @param[in] reserved Extensibility parameter (currently unused and must be 0)
 * @param[out] delegate Pointer where to store the function pointer result
 * @return 0 on success, otherwise failure
 */
typedef int (DELEGATE_CALLTYPE *load)(
	const char_t*	assemblyPath,
	const char_t*	typeName,
	const char_t*	methodName,
	const char_t*	delegateTypeName,
	void*			reserved,
	void**		    delegate
);

/**
 * @brief Signature of delegate returned by
 * coreclr_delegate_type::component_entry_point
 *
 * @param[in] arg Pointer to the argument
 * @param[in] arg_size_in_bytes Size of the argument in bytes
 * @return 0 on success, otherwise failure
 */
typedef int (DELEGATE_CALLTYPE *component_entry_point_fn)(
	void*		arg,
	int32_t     arg_size_in_bytes
);

/**
 * @brief Signature of delegate returned by
 * coreclr_delegate_type::get_function_pointer
 * 
 * @param[in] typeName Assembly qualified type name
 * @param[in] methodName Public static method name compatible with delegateType
 * @param[in] delegateTypeName Assembly qualified delegate type name or null,
 * or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
 * the UnmanagedCallersOnlyAttribute.
 * @param[in] loadContext Extensibility parameter (currently unused and must be 0)
 * @param[in] reserved Extensibility parameter (currently unused and must be 0)
 * @param[out] delegate Pointer where to store the function pointer result
 * @return 0 on success, otherwise failure
 */
typedef int (DELEGATE_CALLTYPE *get_function_pointer_fn)(
	const char_t*	typeName,
	const char_t*	methodName,
	const char_t*	delegateTypeName,
	void*			loadContext,
	void*			reserved,
	void**		    delegate
);

} // namespace MCS