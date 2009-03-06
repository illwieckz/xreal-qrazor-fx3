#ifdef __cplusplus
extern          "C"
{
#endif

	void           *VM_LoadLLVM(vm_t * vm, intptr_t(*systemcalls) (intptr_t, ...));
	void            VM_UnloadLLVM(void *llvmModuleProvider);

#ifdef __cplusplus
}
#endif
