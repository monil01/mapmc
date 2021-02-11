package org.jllvm;

import org.jllvm.bindings.Core;
import org.jllvm.bindings.LLVMTypeKind;
import org.jllvm.bindings.SWIGTYPE_p_LLVMOpaqueType;

/* Implements all methods from Core.h for integer types. */
public class LLVMIntegerType extends LLVMType {
	// Avoid using global context: see LLVMContext documentation.
	//public static LLVMIntegerType i64 = new LLVMIntegerType(Core.LLVMInt64Type());
	//public static LLVMIntegerType i32 = new LLVMIntegerType(Core.LLVMInt32Type());
	//public static LLVMIntegerType i16 = new LLVMIntegerType(Core.LLVMInt16Type());
	//public static LLVMIntegerType i8 = new LLVMIntegerType(Core.LLVMInt8Type());
	//public static LLVMIntegerType i1 = new LLVMIntegerType(Core.LLVMInt1Type());
	
	// Avoid using global context: see LLVMContext documentation.
	//public static LLVMIntegerType get(long NumBits) {
	//	return (LLVMIntegerType)LLVMType.getType(Core.LLVMIntType(NumBits));
	//}
	
	public static LLVMIntegerType get(LLVMContext ctxt, long NumBits) {
		return (LLVMIntegerType)LLVMType.getType(Core.LLVMIntTypeInContext(ctxt.getInstance(),NumBits));
	}
	
	public long getWidth() {
		return Core.LLVMGetIntTypeWidth(instance);
	}
	
	@Override
	public long getPrimitiveSizeInBits() {
		return getWidth();
	}
	
	public LLVMIntegerType(SWIGTYPE_p_LLVMOpaqueType tr) {
		super(tr);
		assert(getTypeKind() == LLVMTypeKind.LLVMIntegerTypeKind);
	}
	
	public String toString() {
		return "i" + getWidth();
	}
	public String toStringForIntrinsic() {
		return toString();
	}
}
