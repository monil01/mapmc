/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.jllvm.bindings;

public class ltoJNI {
  public final static native long new_StringArray(int jarg1);
  public final static native void delete_StringArray(long jarg1);
  public final static native String StringArray_getitem(long jarg1, int jarg2);
  public final static native void StringArray_setitem(long jarg1, int jarg2, String jarg3);
  public final static native int LTO_H_get();
  public final static native int LTO_API_VERSION_get();
  public final static native int LTO_SYMBOL_ALIGNMENT_MASK_get();
  public final static native int LTO_SYMBOL_PERMISSIONS_MASK_get();
  public final static native int LTO_SYMBOL_PERMISSIONS_CODE_get();
  public final static native int LTO_SYMBOL_PERMISSIONS_DATA_get();
  public final static native int LTO_SYMBOL_PERMISSIONS_RODATA_get();
  public final static native int LTO_SYMBOL_DEFINITION_MASK_get();
  public final static native int LTO_SYMBOL_DEFINITION_REGULAR_get();
  public final static native int LTO_SYMBOL_DEFINITION_TENTATIVE_get();
  public final static native int LTO_SYMBOL_DEFINITION_WEAK_get();
  public final static native int LTO_SYMBOL_DEFINITION_UNDEFINED_get();
  public final static native int LTO_SYMBOL_DEFINITION_WEAKUNDEF_get();
  public final static native int LTO_SYMBOL_SCOPE_MASK_get();
  public final static native int LTO_SYMBOL_SCOPE_INTERNAL_get();
  public final static native int LTO_SYMBOL_SCOPE_HIDDEN_get();
  public final static native int LTO_SYMBOL_SCOPE_PROTECTED_get();
  public final static native int LTO_SYMBOL_SCOPE_DEFAULT_get();
  public final static native int LTO_SYMBOL_SCOPE_DEFAULT_CAN_BE_HIDDEN_get();
  public final static native int LTO_DEBUG_MODEL_NONE_get();
  public final static native int LTO_DEBUG_MODEL_DWARF_get();
  public final static native int LTO_CODEGEN_PIC_MODEL_STATIC_get();
  public final static native int LTO_CODEGEN_PIC_MODEL_DYNAMIC_get();
  public final static native int LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC_get();
  public final static native String lto_get_version();
  public final static native String lto_get_error_message();
  public final static native boolean lto_module_is_object_file(String jarg1);
  public final static native boolean lto_module_is_object_file_for_target(String jarg1, String jarg2);
  public final static native boolean lto_module_is_object_file_in_memory(long jarg1, long jarg2);
  public final static native boolean lto_module_is_object_file_in_memory_for_target(long jarg1, long jarg2, String jarg3);
  public final static native long lto_module_create(String jarg1);
  public final static native long lto_module_create_from_memory(long jarg1, long jarg2);
  public final static native long lto_module_create_from_fd(int jarg1, String jarg2, long jarg3);
  public final static native long lto_module_create_from_fd_at_offset(int jarg1, String jarg2, long jarg3, long jarg4, long jarg5);
  public final static native void lto_module_dispose(long jarg1);
  public final static native String lto_module_get_target_triple(long jarg1);
  public final static native void lto_module_set_target_triple(long jarg1, String jarg2);
  public final static native long lto_module_get_num_symbols(long jarg1);
  public final static native String lto_module_get_symbol_name(long jarg1, long jarg2);
  public final static native int lto_module_get_symbol_attribute(long jarg1, long jarg2);
  public final static native long lto_codegen_create();
  public final static native void lto_codegen_dispose(long jarg1);
  public final static native boolean lto_codegen_add_module(long jarg1, long jarg2);
  public final static native boolean lto_codegen_set_debug_model(long jarg1, int jarg2);
  public final static native boolean lto_codegen_set_pic_model(long jarg1, int jarg2);
  public final static native void lto_codegen_set_cpu(long jarg1, String jarg2);
  public final static native void lto_codegen_set_assembler_path(long jarg1, String jarg2);
  public final static native void lto_codegen_set_assembler_args(long jarg1, long jarg2, int jarg3);
  public final static native void lto_codegen_add_must_preserve_symbol(long jarg1, String jarg2);
  public final static native boolean lto_codegen_write_merged_modules(long jarg1, String jarg2);
  public final static native long lto_codegen_compile(long jarg1, long jarg2);
  public final static native boolean lto_codegen_compile_to_file(long jarg1, long jarg2);
  public final static native void lto_codegen_debug_options(long jarg1, String jarg2);
}
