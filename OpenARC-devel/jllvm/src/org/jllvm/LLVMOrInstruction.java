package org.jllvm;

import org.jllvm.bindings.Core;
import org.jllvm.bindings.SWIGTYPE_p_LLVMOpaqueValue;

public class LLVMOrInstruction extends LLVMInstruction {
	private static SWIGTYPE_p_LLVMOpaqueValue buildInstruction(LLVMInstructionBuilder builder,LLVMValue lhs,LLVMValue rhs,String name) {
		return Core.LLVMBuildOr(builder.getInstance(),lhs.getInstance(),rhs.getInstance(),name);
	}
	
	public static LLVMUser create(LLVMInstructionBuilder builder,String name,LLVMValue lhs,LLVMValue rhs) {
		return (LLVMUser)LLVMValue.getValue(buildInstruction(builder, lhs, rhs, name));
	}
	
	public LLVMOrInstruction(SWIGTYPE_p_LLVMOpaqueValue c) {
		super(c);
	}
}
