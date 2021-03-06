#ifndef __OPENACC_HEADER__

#define __OPENACC_HEADER__

//#include <string>
#include <stddef.h>

typedef enum {
    acc_device_none = 0,
    acc_device_default = 1,
    acc_device_host = 2,
    acc_device_not_host = 3,
    acc_device_nvidia = 4,
    acc_device_radeon = 5,
    acc_device_gpu = 6,
    acc_device_xeonphi = 7,
    acc_device_current = 8,
    acc_device_altera = 9,
    acc_device_altera_emulator = 10
} acc_device_t;

typedef enum {
    acc_mem_fence_local = 1,
    acc_mem_fence_global = 2
} acc_mem_fence_flags_t;

#define h_void void
#define d_void void
//acc_async_noval should have the same value as DEFAULT_ASYNC_QUEUE
//in openaccrt.h
#define acc_async_noval -1
#define acc_async_sync -3

///////////////////////////////////////////
// OpenACC V1.0 Runtime Library Routines //
///////////////////////////////////////////
extern void acc_init( acc_device_t devtype );
extern int acc_get_num_devices( acc_device_t devtype );
extern void acc_set_device_type( acc_device_t devtype );
extern acc_device_t acc_get_device_type(void);
extern void acc_set_device_num( int devnum, acc_device_t devtype );
extern int acc_get_device_num( acc_device_t devtype );
extern int acc_async_test( int asyncID );
extern int acc_async_test_all();
extern void acc_async_wait( int asyncID ); //renamed to acc_wait()
extern void acc_async_wait_all(); //renamed to acc_wait_all()
extern void acc_shutdown( acc_device_t devtype );
extern int acc_on_device( acc_device_t devtype );
extern d_void* acc_malloc(size_t);
extern void acc_free(d_void* devPtr);

/////////////////////////////////////////////////////////// 
// OpenACC Runtime Library Routines added in Version 2.0 //
/////////////////////////////////////////////////////////// 
//acc_async_wait() and acc_async_wait_all() are renamed to acc_wait() and
//acc_wait_all() in V2.0.
extern void acc_wait( int arg );
extern void acc_wait_all();
extern void acc_wait_async(int arg, int async);
extern void acc_wait_all_async(int async);
extern void* acc_copyin(h_void* hostPtr, size_t size);
extern void* acc_copyin_async(h_void* hostPtr, size_t size, int async);
extern void* acc_pcopyin(h_void* hostPtr, size_t size);
extern void* acc_present_or_copyin(h_void* hostPtr, size_t size);
extern void* acc_create(h_void* hostPtr, size_t size);
extern void* acc_create_async(h_void* hostPtr, size_t size, int async);
extern void* acc_pcreate(h_void* hostPtr, size_t size);
extern void* acc_present_or_create(h_void* hostPtr, size_t size);
extern void acc_copyout(h_void* hostPtr, size_t size);
extern void acc_copyout_async(h_void* hostPtr, size_t size, int async);
extern void acc_delete(h_void* hostPtr, size_t size);
extern void acc_delete_async(h_void* hostPtr, size_t size, int async);
extern void acc_update_device(h_void* hostPtr, size_t size);
extern void acc_update_device_async(h_void* hostPtr, size_t size, int async);
extern void acc_update_self(h_void* hostPtr, size_t size);
extern void acc_update_self_async(h_void* hostPtr, size_t size, int async);
extern void acc_map_data(h_void* hostPtr, d_void* devPtr, size_t size);
extern void acc_unmap_data(h_void* hostPtr);
extern d_void* acc_deviceptr(h_void* hostPtr);
extern h_void* acc_hostptr(d_void* devPtr);
extern int acc_is_present(h_void* hostPtr, size_t size);
extern void acc_memcpy_to_device(d_void* dest, h_void* src, size_t bytes);
extern void acc_memcpy_from_device(h_void* dest, d_void* src, size_t bytes);

/////////////////////////////////////////////////////////// 
// OpenACC Runtime Library Routines added in Version 2.5 //
/////////////////////////////////////////////////////////// 
extern void acc_memcpy_device(d_void* dest, d_void* src, size_t bytes);
extern void acc_memcpy_device_async(d_void* dest, d_void* src, size_t bytes, int async);

/////////////////////////////////////////////////////////// 
// OpenACC Runtime Library Routines added in Version 2.6 //
/////////////////////////////////////////////////////////// 
extern void acc_attach(h_void** hostPtr);
extern void acc_attach_async(h_void** hostPtr, int async);
extern void acc_detach(h_void** hostPtr);
extern void acc_detach_async(h_void** hostPtr, int async);
extern void acc_detach_finalize(h_void** hostPtr);
extern void acc_detach_finalize_async(h_void** hostPtr, int async);
//extern size_t acc_get_property(int devicenum, acc_device_t devicetype, acc_device_property_t property);
//extern const char* acc_get_property_string(int devicenum, acc_device_t devicetype, acc_device_property_t property);

//////////////////////////////////////////////////////////////////////
// Experimental OpenACC Runtime Library Routines for Unified Memory //
// (Currently, these work only for specific versions of CUDA GPUs.) //
//////////////////////////////////////////////////////////////////////
extern void* acc_copyin_unified(h_void* hostPtr, size_t size);
extern void* acc_pcopyin_unified(h_void* hostPtr, size_t size);
extern void* acc_present_or_copyin_unified(h_void* hostPtr, size_t size);
extern void* acc_create_unified(h_void* hostPtr, size_t size);
extern void* acc_pcreate_unified(h_void* hostPtr, size_t size);
extern void* acc_present_or_create_unified(h_void* hostPtr, size_t size);
extern void acc_copyout_unified(h_void* hostPtr, size_t size);
extern void acc_delete_unified(h_void* hostPtr, size_t size);

/////////////////////////////////////////////////////////////////
// Additional OpenACC Runtime Library Routines Used by OpenARC //
/////////////////////////////////////////////////////////////////
extern void* acc_copyin_const(h_void* hostPtr, size_t size);
extern void* acc_pcopyin_const(h_void* hostPtr, size_t size);
extern void* acc_present_or_copyin_const(h_void* hostPtr, size_t size);
extern void* acc_create_const(h_void* hostPtr, size_t size);
extern void* acc_pcreate_const(h_void* hostPtr, size_t size);
extern void* acc_present_or_create_const(h_void* hostPtr, size_t size);
extern void* acc_copyin_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void* acc_pcopyin_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void* acc_present_or_copyin_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void* acc_create_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void* acc_pcreate_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void* acc_present_or_create_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void acc_copyout_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void acc_delete_async_wait(h_void* hostPtr, size_t size, int async, int arg);
extern void acc_map_data_forced(h_void* hostPtr, d_void* devPtr, size_t size);

///////////////////////////////////////////////
// Internal OpenARC Runtime Library Routines //
///////////////////////////////////////////////
extern int get_thread_id();
extern void HI_set_context();
extern void HI_enter_subregion(const char *label);
extern void HI_exit_subregion(const char *label);

#endif
