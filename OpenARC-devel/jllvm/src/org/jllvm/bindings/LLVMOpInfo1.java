/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.jllvm.bindings;

public class LLVMOpInfo1 {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected LLVMOpInfo1(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(LLVMOpInfo1 obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        DisassemblerJNI.delete_LLVMOpInfo1(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setAddSymbol(LLVMOpInfoSymbol1 value) {
    DisassemblerJNI.LLVMOpInfo1_AddSymbol_set(swigCPtr, this, LLVMOpInfoSymbol1.getCPtr(value), value);
  }

  public LLVMOpInfoSymbol1 getAddSymbol() {
    long cPtr = DisassemblerJNI.LLVMOpInfo1_AddSymbol_get(swigCPtr, this);
    return (cPtr == 0) ? null : new LLVMOpInfoSymbol1(cPtr, false);
  }

  public void setSubtractSymbol(LLVMOpInfoSymbol1 value) {
    DisassemblerJNI.LLVMOpInfo1_SubtractSymbol_set(swigCPtr, this, LLVMOpInfoSymbol1.getCPtr(value), value);
  }

  public LLVMOpInfoSymbol1 getSubtractSymbol() {
    long cPtr = DisassemblerJNI.LLVMOpInfo1_SubtractSymbol_get(swigCPtr, this);
    return (cPtr == 0) ? null : new LLVMOpInfoSymbol1(cPtr, false);
  }

  public void setValue(SWIGTYPE_p_uint64_t value) {
    DisassemblerJNI.LLVMOpInfo1_Value_set(swigCPtr, this, SWIGTYPE_p_uint64_t.getCPtr(value));
  }

  public SWIGTYPE_p_uint64_t getValue() {
    return new SWIGTYPE_p_uint64_t(DisassemblerJNI.LLVMOpInfo1_Value_get(swigCPtr, this), true);
  }

  public void setVariantKind(SWIGTYPE_p_uint64_t value) {
    DisassemblerJNI.LLVMOpInfo1_VariantKind_set(swigCPtr, this, SWIGTYPE_p_uint64_t.getCPtr(value));
  }

  public SWIGTYPE_p_uint64_t getVariantKind() {
    return new SWIGTYPE_p_uint64_t(DisassemblerJNI.LLVMOpInfo1_VariantKind_get(swigCPtr, this), true);
  }

  public LLVMOpInfo1() {
    this(DisassemblerJNI.new_LLVMOpInfo1(), true);
  }

}
