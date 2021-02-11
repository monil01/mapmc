/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.jllvm.bindings;

public final class LLVMByteOrdering {
  public final static LLVMByteOrdering LLVMBigEndian = new LLVMByteOrdering("LLVMBigEndian");
  public final static LLVMByteOrdering LLVMLittleEndian = new LLVMByteOrdering("LLVMLittleEndian");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static LLVMByteOrdering swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + LLVMByteOrdering.class + " with value " + swigValue);
  }

  private LLVMByteOrdering(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private LLVMByteOrdering(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private LLVMByteOrdering(String swigName, LLVMByteOrdering swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static LLVMByteOrdering[] swigValues = { LLVMBigEndian, LLVMLittleEndian };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

