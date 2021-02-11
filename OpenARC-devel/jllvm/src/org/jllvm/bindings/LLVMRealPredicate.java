/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.jllvm.bindings;

public final class LLVMRealPredicate {
  public final static LLVMRealPredicate LLVMRealPredicateFalse = new LLVMRealPredicate("LLVMRealPredicateFalse");
  public final static LLVMRealPredicate LLVMRealOEQ = new LLVMRealPredicate("LLVMRealOEQ");
  public final static LLVMRealPredicate LLVMRealOGT = new LLVMRealPredicate("LLVMRealOGT");
  public final static LLVMRealPredicate LLVMRealOGE = new LLVMRealPredicate("LLVMRealOGE");
  public final static LLVMRealPredicate LLVMRealOLT = new LLVMRealPredicate("LLVMRealOLT");
  public final static LLVMRealPredicate LLVMRealOLE = new LLVMRealPredicate("LLVMRealOLE");
  public final static LLVMRealPredicate LLVMRealONE = new LLVMRealPredicate("LLVMRealONE");
  public final static LLVMRealPredicate LLVMRealORD = new LLVMRealPredicate("LLVMRealORD");
  public final static LLVMRealPredicate LLVMRealUNO = new LLVMRealPredicate("LLVMRealUNO");
  public final static LLVMRealPredicate LLVMRealUEQ = new LLVMRealPredicate("LLVMRealUEQ");
  public final static LLVMRealPredicate LLVMRealUGT = new LLVMRealPredicate("LLVMRealUGT");
  public final static LLVMRealPredicate LLVMRealUGE = new LLVMRealPredicate("LLVMRealUGE");
  public final static LLVMRealPredicate LLVMRealULT = new LLVMRealPredicate("LLVMRealULT");
  public final static LLVMRealPredicate LLVMRealULE = new LLVMRealPredicate("LLVMRealULE");
  public final static LLVMRealPredicate LLVMRealUNE = new LLVMRealPredicate("LLVMRealUNE");
  public final static LLVMRealPredicate LLVMRealPredicateTrue = new LLVMRealPredicate("LLVMRealPredicateTrue");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static LLVMRealPredicate swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + LLVMRealPredicate.class + " with value " + swigValue);
  }

  private LLVMRealPredicate(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private LLVMRealPredicate(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private LLVMRealPredicate(String swigName, LLVMRealPredicate swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static LLVMRealPredicate[] swigValues = { LLVMRealPredicateFalse, LLVMRealOEQ, LLVMRealOGT, LLVMRealOGE, LLVMRealOLT, LLVMRealOLE, LLVMRealONE, LLVMRealORD, LLVMRealUNO, LLVMRealUEQ, LLVMRealUGT, LLVMRealUGE, LLVMRealULT, LLVMRealULE, LLVMRealUNE, LLVMRealPredicateTrue };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

