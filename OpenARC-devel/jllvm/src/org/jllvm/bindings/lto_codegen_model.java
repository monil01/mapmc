/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.jllvm.bindings;

public final class lto_codegen_model {
  public final static lto_codegen_model LTO_CODEGEN_PIC_MODEL_STATIC = new lto_codegen_model("LTO_CODEGEN_PIC_MODEL_STATIC", ltoJNI.LTO_CODEGEN_PIC_MODEL_STATIC_get());
  public final static lto_codegen_model LTO_CODEGEN_PIC_MODEL_DYNAMIC = new lto_codegen_model("LTO_CODEGEN_PIC_MODEL_DYNAMIC", ltoJNI.LTO_CODEGEN_PIC_MODEL_DYNAMIC_get());
  public final static lto_codegen_model LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC = new lto_codegen_model("LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC", ltoJNI.LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static lto_codegen_model swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + lto_codegen_model.class + " with value " + swigValue);
  }

  private lto_codegen_model(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private lto_codegen_model(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private lto_codegen_model(String swigName, lto_codegen_model swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static lto_codegen_model[] swigValues = { LTO_CODEGEN_PIC_MODEL_STATIC, LTO_CODEGEN_PIC_MODEL_DYNAMIC, LTO_CODEGEN_PIC_MODEL_DYNAMIC_NO_PIC };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

