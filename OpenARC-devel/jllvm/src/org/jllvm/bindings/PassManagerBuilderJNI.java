/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.10
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.jllvm.bindings;

public class PassManagerBuilderJNI {
  public final static native long LLVMPassManagerBuilderCreate();
  public final static native void LLVMPassManagerBuilderDispose(long jarg1);
  public final static native void LLVMPassManagerBuilderSetOptLevel(long jarg1, long jarg2);
  public final static native void LLVMPassManagerBuilderSetSizeLevel(long jarg1, long jarg2);
  public final static native void LLVMPassManagerBuilderSetDisableUnitAtATime(long jarg1, int jarg2);
  public final static native void LLVMPassManagerBuilderSetDisableUnrollLoops(long jarg1, int jarg2);
  public final static native void LLVMPassManagerBuilderSetDisableSimplifyLibCalls(long jarg1, int jarg2);
  public final static native void LLVMPassManagerBuilderUseInlinerWithThreshold(long jarg1, long jarg2);
  public final static native void LLVMPassManagerBuilderPopulateFunctionPassManager(long jarg1, long jarg2);
  public final static native void LLVMPassManagerBuilderPopulateModulePassManager(long jarg1, long jarg2);
  public final static native void LLVMPassManagerBuilderPopulateLTOPassManager(long jarg1, long jarg2, boolean jarg3, boolean jarg4);
}
