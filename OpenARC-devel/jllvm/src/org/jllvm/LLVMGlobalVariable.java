package org.jllvm;

import org.jllvm.bindings.Core;
import org.jllvm.bindings.SWIGTYPE_p_LLVMOpaqueValue;

public class LLVMGlobalVariable extends LLVMGlobalValue {
	public LLVMGlobalVariable(SWIGTYPE_p_LLVMOpaqueValue c) {
		super(c);
	}
	
	public static LLVMGlobalVariable getGlobalVariable(SWIGTYPE_p_LLVMOpaqueValue c) {
		return c == null ? new LLVMGlobalVariable(null)
		                 : (LLVMGlobalVariable)LLVMValue.getValue(c);
	}
	
	public LLVMGlobalVariable getNextGlobal() {
		return getGlobalVariable(Core.LLVMGetNextGlobal(instance));
	}
	
	public LLVMGlobalVariable getPreviousGlobal() {
		return getGlobalVariable(Core.LLVMGetPreviousGlobal(instance));
	}
	
	public LLVMConstant getInitializer() {
		return (LLVMConstant)LLVMValue.getValue(Core.LLVMGetInitializer(instance));
	}
	
	public void setInitializer(LLVMConstant val) {
		assert(typeOf() instanceof LLVMPointerType);
		assert(val.typeOf() == ((LLVMPointerType)typeOf()).getElementType());
		Core.LLVMSetInitializer(instance,val.getInstance());
	}
	
	public boolean isThreadLocal() {
		return (Core.LLVMIsThreadLocal(instance) != 0);
	}
	
	public void setThreadLocal(boolean threadLocal) {
		Core.LLVMSetThreadLocal(instance,threadLocal ? 1 : 0);
	}
	
	public boolean isConstant() {
		return (Core.LLVMIsGlobalConstant(instance) != 0);
	}
	
	public void setConstant(boolean constant) {
		Core.LLVMSetGlobalConstant(instance,constant ? 1 : 0);
	}
	
	public void delete() {
		Core.LLVMDeleteGlobal(instance);
	}
}
