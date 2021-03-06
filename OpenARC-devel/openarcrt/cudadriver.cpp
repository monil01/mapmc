#include "openacc.h"
#include "openaccrt_ext.h"
#include <sstream>
////////////////////////////////////////////////////////
//Current implementation works with CUDA5.0 or later. //
////////////////////////////////////////////////////////
//See HI_synchronize() to see the effect of using
//blocking streams.
#define USE_BLOCKING_STREAMS
#define SHOW_ERROR_CODE

//#define INIT_DEBUG
#ifdef INIT_DEBUG
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

//[DEBUG] commented out since they are no more static.
//std::map<std::string, CUfunction> CudaDriver::kernelMap;
//std::set<std::string> CudaDriver::kernelNameSet;
std::map<CUdeviceptr,int> CudaDriver::pinnedHostMemCounter;
std::vector<const void *> CudaDriver::hostMemToUnpin;

const char * cuda_error_code(CUresult err) {
	static std::string str = "";
#ifdef SHOW_ERROR_CODE
	/* Error Codes */
	switch ( err ) {
		case CUDA_SUCCESS: { str = "CUDA_SUCCESS"; break; } 
		case CUDA_ERROR_INVALID_VALUE: { str = "CUDA_ERROR_INVALID_VALUE"; break; } 
		case CUDA_ERROR_OUT_OF_MEMORY: { str = "CUDA_ERROR_OUT_OF_MEMORY"; break; }
		case CUDA_ERROR_NOT_INITIALIZED: { str = "CUDA_ERROR_NOT_INITIALIZED"; break; }
		case CUDA_ERROR_DEINITIALIZED: { str = "CUDA_ERROR_DEINITIALIZED"; break; }
		case CUDA_ERROR_PROFILER_DISABLED: { str = "CUDA_ERROR_PROFILER_DISABLED"; break; }
		case CUDA_ERROR_PROFILER_NOT_INITIALIZED: { str = "CUDA_ERROR_PROFILER_NOT_INITIALIZED"; break; }
		case CUDA_ERROR_PROFILER_ALREADY_STARTED: { str = "CUDA_ERROR_PROFILER_ALREADY_STARTED"; break; }
		case CUDA_ERROR_PROFILER_ALREADY_STOPPED: { str = "CUDA_ERROR_PROFILER_ALREADY_STOPPED"; break; }
		case CUDA_ERROR_NO_DEVICE: { str = "CUDA_ERROR_NO_DEVICE"; break; }
		case CUDA_ERROR_INVALID_DEVICE: { str = "CUDA_ERROR_INVALID_DEVICE"; break; }
		case CUDA_ERROR_INVALID_IMAGE: { str = "CUDA_ERROR_INVALID_IMAGE"; break; }
		case CUDA_ERROR_INVALID_CONTEXT: { str = "CUDA_ERROR_INVALID_CONTEXT"; break; }
		case CUDA_ERROR_CONTEXT_ALREADY_CURRENT: { str = "CUDA_ERROR_CONTEXT_ALREADY_CURRENT"; break; }
		case CUDA_ERROR_MAP_FAILED: { str = "CUDA_ERROR_MAP_FAILED"; break; }
		case CUDA_ERROR_UNMAP_FAILED: { str = "CUDA_ERROR_UNMAP_FAILED"; break; }
		case CUDA_ERROR_ARRAY_IS_MAPPED: { str = "CUDA_ERROR_ARRAY_IS_MAPPED"; break; }
		case CUDA_ERROR_ALREADY_MAPPED: { str = "CUDA_ERROR_ALREADY_MAPPED"; break; }
		case CUDA_ERROR_NO_BINARY_FOR_GPU: { str = "CUDA_ERROR_NO_BINARY_FOR_GPU"; break; }
		case CUDA_ERROR_ALREADY_ACQUIRED: { str = "CUDA_ERROR_ALREADY_ACQUIRED"; break; }
		case CUDA_ERROR_NOT_MAPPED: { str = "CUDA_ERROR_NOT_MAPPED"; break; }
		case CUDA_ERROR_NOT_MAPPED_AS_ARRAY: { str = "CUDA_ERROR_NOT_MAPPED_AS_ARRAY"; break; }
		case CUDA_ERROR_NOT_MAPPED_AS_POINTER: { str = "CUDA_ERROR_NOT_MAPPED_AS_POINTER"; break; }
		case CUDA_ERROR_ECC_UNCORRECTABLE: { str = "CUDA_ERROR_ECC_UNCORRECTABLE"; break; }
		case CUDA_ERROR_UNSUPPORTED_LIMIT: { str = "CUDA_ERROR_UNSUPPORTED_LIMIT"; break; }
		case CUDA_ERROR_CONTEXT_ALREADY_IN_USE: { str = "CUDA_ERROR_CONTEXT_ALREADY_IN_USE"; break; }
		case CUDA_ERROR_PEER_ACCESS_UNSUPPORTED: { str = "CUDA_ERROR_PEER_ACCESS_UNSUPPORTED"; break; }
		case CUDA_ERROR_INVALID_PTX: { str = "CUDA_ERROR_INVALID_PTX"; break; }
//		case CUDA_ERROR_INVALID_GRAPHICS_CONTEXT: { str = "CUDA_ERROR_INVALID_GRAPHICS_CONTEXT"; break; }
		case CUDA_ERROR_INVALID_SOURCE: { str = "CUDA_ERROR_INVALID_SOURCE"; break; }
		case CUDA_ERROR_FILE_NOT_FOUND: { str = "CUDA_ERROR_FILE_NOT_FOUND"; break; }
		case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND: { str = "CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND"; break; }
		case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED: { str = "CUDA_ERROR_SHARED_OBJECT_INIT_FAILED"; break; }
		case CUDA_ERROR_OPERATING_SYSTEM: { str = "CUDA_ERROR_OPERATING_SYSTEM"; break; }
		case CUDA_ERROR_INVALID_HANDLE: { str = "CUDA_ERROR_INVALID_HANDLE"; break; }
		case CUDA_ERROR_NOT_FOUND: { str = "CUDA_ERROR_NOT_FOUND"; break; }
		case CUDA_ERROR_NOT_READY: { str = "CUDA_ERROR_NOT_READY"; break; }
		case CUDA_ERROR_ILLEGAL_ADDRESS: { str = "CUDA_ERROR_ILLEGAL_ADDRESS"; break; }
		case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES: { str = "CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES"; break; }
		case CUDA_ERROR_LAUNCH_TIMEOUT: { str = "CUDA_ERROR_LAUNCH_TIMEOUT"; break; }
		case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING: { str = "CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING"; break; }
		case CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED: { str = "CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED"; break; }
		case CUDA_ERROR_PEER_ACCESS_NOT_ENABLED: { str = "CUDA_ERROR_PEER_ACCESS_NOT_ENABLED"; break; }
		case CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE: { str = "CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE"; break; }
		case CUDA_ERROR_CONTEXT_IS_DESTROYED: { str = "CUDA_ERROR_CONTEXT_IS_DESTROYED"; break; }
		case CUDA_ERROR_ASSERT: { str = "CUDA_ERROR_ASSERT"; break; }
		case CUDA_ERROR_TOO_MANY_PEERS: { str = "CUDA_ERROR_TOO_MANY_PEERS"; break; }
		case CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED: { str = "CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED"; break; }
		case CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED: { str = "CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED"; break; }
		case CUDA_ERROR_HARDWARE_STACK_ERROR: { str = "CUDA_ERROR_HARDWARE_STACK_ERROR"; break; }
		case CUDA_ERROR_ILLEGAL_INSTRUCTION: { str = "CUDA_ERROR_ILLEGAL_INSTRUCTION"; break; }
		case CUDA_ERROR_MISALIGNED_ADDRESS: { str = "CUDA_ERROR_MISALIGNED_ADDRESS"; break; }
		case CUDA_ERROR_INVALID_ADDRESS_SPACE: { str = "CUDA_ERROR_INVALID_ADDRESS_SPACE"; break; }
		case CUDA_ERROR_INVALID_PC: { str = "CUDA_ERROR_INVALID_PC"; break; }
		case CUDA_ERROR_LAUNCH_FAILED: { str = "CUDA_ERROR_LAUNCH_FAILED"; break; }
		case CUDA_ERROR_NOT_PERMITTED: { str = "CUDA_ERROR_NOT_PERMITTED"; break; }
		case CUDA_ERROR_NOT_SUPPORTED: { str = "CUDA_ERROR_NOT_SUPPORTED"; break; }
		default: { str = "CUDA_ERROR_UNKNOWN"; break; }
	}
#endif
	return str.c_str();
}

///////////////////////////
// Device Initialization //
///////////////////////////
CudaDriver::CudaDriver(acc_device_t devType, int devNum, std::set<std::string>kernelNames, HostConf_t *conf, int numDevices, const char *baseFileName) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::CudaDriver(%s, %d)\n", HI_get_device_type_string(devType), devNum);
	}
#endif
    dev = devType;
    device_num = devNum;
	num_devices = numDevices;
	fileNameBase = std::string(baseFileName);
	cuContext = NULL;
	current_mempool_size = 0;

    for (std::set<std::string>::iterator it = kernelNames.begin() ; it != kernelNames.end(); ++it) {
        kernelNameSet.insert(*it);
    }
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::CudaDriver(%s, %d)\n", HI_get_device_type_string(devType), devNum);
	}
#endif
}

HI_error_t CudaDriver::init(int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::init()\n");
	}
#endif

    CUresult err;
    /*
    cudaDeviceProp deviceProp;
    cudaError_t cuResult = cudaGetDeviceProperties(&deviceProp, device_num);
    */

    cuInit(0);
    cuDeviceGet(&cuDevice, device_num);
    char name[256];
    cuDeviceGetName(name, sizeof(name), cuDevice);

    HostConf_t * tconf = getHostConf(threadID);
    int thread_id = tconf->threadID;
#ifdef _OPENARC_PROFILE_
    fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::init(): host Thread %d initializes device %d: %s\n", thread_id, device_num, name);
#endif
    cuDeviceGet(&cuDevice, device_num);

#if CUDA_VERSION >= 5000
    cuDeviceGetAttribute (&compute_capability_major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, cuDevice);
    cuDeviceGetAttribute (&compute_capability_minor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, cuDevice);
#else
    cuDeviceComputeCapability(&compute_capability_major, &compute_capability_minor, cuDevice);
#endif
#ifdef INIT_DEBUG
    fprintf(stderr, "[DEBUG] Compute capability: %d.%d\n", compute_capability_major, compute_capability_minor);
	fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
	if( compute_capability_major > 2 ) {
		maxGridX = 2147483647; maxGridY = 65535; maxGridZ = 65535;
		maxBlockX = 1024; maxBlockY = 1024; maxBlockZ = 64;
		maxNumThreadsPerBlock = 1024;
	} else {
		maxGridX = 65535; maxGridY = 65535; maxGridZ = 1;
		maxBlockX = 512; maxBlockY = 512; maxBlockZ = 64;
		maxNumThreadsPerBlock = 512;
	}
	max1DTexRefWidth4LM = 134217728;
	current_mempool_size = 0;
	if( tconf->use_unifiedmemory > 0 ) {
#if CUDA_VERSION >= 6000
		cuDeviceGetAttribute(&unifiedMemSupported, CU_DEVICE_ATTRIBUTE_MANAGED_MEMORY, cuDevice);
#else
		unifiedMemSupported = 0;
#endif
	} else {
		unifiedMemSupported = 0;
	}

    std::stringstream ss;
    ss << compute_capability_major;

    ss << compute_capability_minor;
    std::string version = ss.str();

    std::string ptxName = fileNameBase + std::string("_") + version + std::string(".ptx");

    //compile a PTX if it does not already exist
    if( access( ptxName.c_str(), F_OK ) == -1 ) {
        std::string command = std::string("nvcc $OPENARC_JITOPTION -arch=sm_") + version + std::string(" ") + fileNameBase + std::string(".cu -ptx -o ") + ptxName;
        //fprintf(stderr, "Version no. %s\n", version.c_str());
        if( system(command.c_str()) == -1) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] failed to compile the input CUDA kernel file (%s%s); exit!\n", fileNameBase.c_str(), ".cu");
			exit(1);
		}
    }

	//[INFO] If a context is created by the CUDA runtime API, instead of CUDA driver 
	//API, the runtime API uses a hidden API to create what is called a primary 
	//context, which can't be accessed directly by the driver API.
	//
	//If a standard context is created by the driver API, the following CUDA runtime
	//API called by the same host thread will not create the primary context.

	//Check whether the current host thread has a CUDA context.
	err = cuCtxGetCurrent(&cuContext);
    if((err == CUDA_SUCCESS) && (cuContext != NULL)) {
		//If existing, use the current CUDA context.
		cuCtxSetCurrent(cuContext);
	} else {
		//Create a new CUDA context.
		//Default flag (0) uses CU_CTX_SCHED_AUTO, but to make cuCtxSynchronize()
		//blocking, CU_CTX_SCHED_BLOCKING_SYNC should be used instead.
    	//err = cuCtxCreate(&cuContext, 0, cuDevice);
    	err = cuCtxCreate(&cuContext, CU_CTX_SCHED_BLOCKING_SYNC, cuDevice);
    	if(err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] failed to create CUDA context with error %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
	}
#ifdef INIT_DEBUG
    fprintf(stderr, "[DEBUG] A CUDA context is created or loaded.\n");
	fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif

/*
    std::string ptx_source;
    FILE *fp = fopen(ptxName.c_str(), "rb");
    if(fp == NULL) {
        fprintf(stderr, "[ERROR in CudaDriver::init()] failed to open PTX file %s in CUDA (NVIDIA CUDA GPU)\n", ptxName.c_str());
		exit(1);
        //printf("PTX not openend\n");
    }
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    char *buf = new char[file_size+1];
    fseek(fp, 0, SEEK_SET);
    fread(buf, sizeof(char), file_size, fp);
    fclose(fp);
    buf[file_size] = '\0';
    ptx_source = buf;
    delete[] buf;

    //PTX JIT
    const unsigned int jitNumOptions = 2;
    CUjit_option *jitOptions = new CUjit_option[jitNumOptions];
    void **jitOptVals = new void*[jitNumOptions];

    // set up size of compilation log buffer
    jitOptions[0] = CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES;
    int jitLogBufferSize = 1024;
    jitOptVals[0] = (void *)(size_t)jitLogBufferSize;

    // set up pointer to the compilation log buffer
    jitOptions[1] = CU_JIT_INFO_LOG_BUFFER;
    char *jitLogBuffer = new char[jitLogBufferSize];
    jitOptVals[1] = jitLogBuffer;

    // set up pointer to set the Maximum # of registers for a particular kernel
    //jitOptions[2] = CU_JIT_MAX_REGISTERS;
    //int jitRegCount = 32;
    //jitOptVals[2] = (void *)(size_t)jitRegCount;

    //jitOptions[2] = CU_JIT_TARGET;
    //int nullVal = 0;
    //jitOptVals[2] = (void *)(uintptr_t)CU_TARGET_COMPUTE_30;

    err = cuModuleLoadDataEx(&cuModule, ptx_source.c_str(), jitNumOptions, jitOptions, (void **)jitOptVals);
*/
	err = cuModuleLoad(&cuModule, ptxName.c_str());
    if (err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::init()] Module Load FAIL with error = %d (%s)\n", err, cuda_error_code(err));
		exit(1);
    }
#ifdef INIT_DEBUG
    fprintf(stderr, "[DEBUG] CUDA Module is loaded.\n");
	fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
    CUstream s0, s1;
    CUevent e0, e1;
	// CU_STREAM_DEFAULT => create a blocking stream that synchronizes with 
	// the NULL stream.
	// CU_STREAM_NON_BLOCKING => create a non-blocking stream that may run 
	// concurrently with the NULL stream (no implicit synchronization with 
	// the NULL stream).
	//for( int i=0; i<HI_num_hostthreads; i++ ) {
#ifdef USE_BLOCKING_STREAMS
    	err = cuStreamCreate(&s0, CU_STREAM_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	err = cuStreamCreate(&s1, CU_STREAM_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
#else
    	err = cuStreamCreate(&s0, CU_STREAM_NON_BLOCKING);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	err = cuStreamCreate(&s1, CU_STREAM_NON_BLOCKING);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
#endif
    	queueMap[0+thread_id*MAX_NUM_QUEUES_PER_THREAD] = s0;
    	queueMap[1+thread_id*MAX_NUM_QUEUES_PER_THREAD] = s1;
#ifdef INIT_DEBUG
    	fprintf(stderr, "[DEBUG] CUDA Streams are created.\n");
		fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
    	err = cuEventCreate(&e0, CU_EVENT_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] Event Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	std::map<int, CUevent> eventMap;
    	eventMap[0+thread_id*MAX_NUM_QUEUES_PER_THREAD]= e0;
    	err = cuEventCreate(&e1, CU_EVENT_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::init()] Event Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	eventMap[1+thread_id*MAX_NUM_QUEUES_PER_THREAD]= e1;
    	threadQueueEventMap[thread_id] = eventMap;
		masterAddressTableMap[thread_id] = new addresstable_t();
		masterHandleTable[thread_id] = new addressmap_t();
		postponedFreeTableMap[thread_id] = new asyncfreetable_t();
		postponedTempFreeTableMap[thread_id] = new asynctempfreetable_t();
		postponedTempFreeTableMap2[thread_id] = new asynctempfreetable2_t();
		memPoolMap[thread_id] = new memPool_t();
		tempMallocSizeMap[thread_id] = new sizemap_t();
#ifdef INIT_DEBUG
    	fprintf(stderr, "[DEBUG] CUDA events are created.\n");
		fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
	//}


    createKernelArgMap(thread_id);

    init_done = 1;
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::init()\n");
	}
#endif
    return HI_success;
}

HI_error_t CudaDriver::createKernelArgMap(int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::createKernelArgMap()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUresult err;
    err = cuCtxSetCurrent(cuContext);
    if (err != CUDA_SUCCESS) {
		fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Set Context FAIL with error = %d (%s)\n", err, cuda_error_code(err));
		exit(1);
	}
    std::map<std::string, kernelParams_t*> kernelArgs;
    std::map<std::string, CUfunction> kernelMap;
    for(std::set<std::string>::iterator it=kernelNameSet.begin(); it!=kernelNameSet.end(); ++it) {
        // Create argument mapping for the kernel
        const char *kernelName = (*it).c_str();
        //fprintf(stderr, "[INFO in CudaDriver::createKernelArgMap()] Function to load: %s\n", kernelName);
        CUfunction cuFunc;
		kernelParams_t *kernelParams = new kernelParams_t;
		kernelParams->num_args = 0;
        kernelArgs.insert(std::pair<std::string, kernelParams_t*>(std::string(kernelName), kernelParams));
        err = cuModuleGetFunction(&cuFunc, cuModule, kernelName);
        if (err != CUDA_SUCCESS) {
            fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Function Load FAIL on %s with error = %d (%s)\n", kernelName, err, cuda_error_code(err));
			exit(1);
        }
        kernelMap[*it] = cuFunc;
    }

    tconf->kernelArgsMap[this] = kernelArgs;
    tconf->kernelsMap[this]=kernelMap;

	int thread_id = tconf->threadID;
	if( queueMap.count(0+thread_id*MAX_NUM_QUEUES_PER_THREAD) == 0 ) {
    	CUstream s0, s1;
#ifdef USE_BLOCKING_STREAMS
    	err = cuStreamCreate(&s0, CU_STREAM_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	err = cuStreamCreate(&s1, CU_STREAM_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
#else
    	err = cuStreamCreate(&s0, CU_STREAM_NON_BLOCKING);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	err = cuStreamCreate(&s1, CU_STREAM_NON_BLOCKING);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Stream Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
#endif
    	queueMap[0+thread_id*MAX_NUM_QUEUES_PER_THREAD] = s0;
    	queueMap[1+thread_id*MAX_NUM_QUEUES_PER_THREAD] = s1;
#ifdef INIT_DEBUG
    	fprintf(stderr, "[DEBUG] CUDA Streams are created.\n");
		fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
	}

	if( (threadQueueEventMap.count(thread_id) == 0) || (threadQueueEventMap.at(thread_id).count(thread_id*MAX_NUM_QUEUES_PER_THREAD) == 0) ) {
    	CUevent e0, e1;
    	err = cuEventCreate(&e0, CU_EVENT_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Event Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	std::map<int, CUevent> eventMap;
    	eventMap[0+thread_id*MAX_NUM_QUEUES_PER_THREAD]= e0;
    	err = cuEventCreate(&e1, CU_EVENT_DEFAULT);
#ifdef _OPENARC_PROFILE_
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::createKernelArgMap()] Event Create FAIL with error = %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
#endif
    	eventMap[1+thread_id*MAX_NUM_QUEUES_PER_THREAD]= e1;
    	threadQueueEventMap[thread_id] = eventMap;
		masterAddressTableMap[thread_id] = new addresstable_t();
		masterHandleTable[thread_id] = new addressmap_t();
		postponedFreeTableMap[thread_id] = new asyncfreetable_t();
		postponedTempFreeTableMap[thread_id] = new asynctempfreetable_t();
		postponedTempFreeTableMap2[thread_id] = new asynctempfreetable2_t();
		memPoolMap[thread_id] = new memPool_t();
		tempMallocSizeMap[thread_id] = new sizemap_t();
#ifdef INIT_DEBUG
    	fprintf(stderr, "[DEBUG] CUDA events are created.\n");
		fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
	}

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::createKernelArgMap()\n");
	}
#endif
    return HI_success;
}

HI_error_t CudaDriver::HI_register_kernels(std::set<std::string> kernelNames, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_register_kernels()\n");
	}
#endif
#ifdef INIT_DEBUG
    fprintf(stderr, "[DEBUG] call HI_register_kernel().\n");
	fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUresult err;
    for (std::set<std::string>::iterator it = kernelNames.begin() ; it != kernelNames.end(); ++it) {
		if( kernelNameSet.count(*it) == 0 ) {
        	// Create argument mapping for the kernel
        	const char *kernelName = (*it).c_str();
        	CUfunction cuFunc;
			kernelParams_t *kernelParams = new kernelParams_t;
			kernelParams->num_args = 0;
        	(tconf->kernelArgsMap[this]).insert(std::pair<std::string, kernelParams_t*>(std::string(kernelName), kernelParams));
        	err = cuModuleGetFunction(&cuFunc, cuModule, kernelName);
        	if (err != CUDA_SUCCESS) {
            	fprintf(stderr, "[ERROR in CudaDriver::HI_register_kernels()] Function Load FAIL on %s with error = %d (%s)\n", kernelName, err, cuda_error_code(err));
				exit(1);
        	}
        	(tconf->kernelsMap[this])[*it] = cuFunc;
		}
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_set_device_num);
#else
#ifdef _OPENMP
        #pragma omp critical(acc_set_device_num_critical)
#endif
#endif
    for (std::set<std::string>::iterator it = kernelNames.begin() ; it != kernelNames.end(); ++it) {
		if( kernelNameSet.count(*it) == 0 ) {
			//Add a new kernel name.
        	kernelNameSet.insert(*it);
		}
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_set_device_num);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_register_kernels()\n");
	}
#endif
    return HI_success;
}

int CudaDriver::HI_get_num_devices(acc_device_t devType, int threadID) {
    int numDevices;
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_get_num_devices()\n");
	}
#endif
    //cudaGetDeviceCount(&numDevices);
    cuInit(0);
	if( devType == acc_device_gpu ) {
    	cuDeviceGetCount(&numDevices);
	} else {
		numDevices = 0;
	}
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_get_num_devices()\n");
	}
#endif
    return numDevices;
}


HI_error_t CudaDriver::destroy(int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::destroy()\n");
	}
#endif
    CUresult err = cuCtxDestroy(cuContext);
    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::destroy()] failed to destroy CUDA context with error %d (%s)\n", err, cuda_error_code(err));
        return HI_error;
    }
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::destroy()\n");
	}
#endif
    return HI_success;
}

// Pin host memory
HI_error_t CudaDriver::HI_pin_host_memory(const void* hostPtr, size_t size, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_pin_host_memory()\n");
	}
#endif
    HI_error_t result = HI_success;
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
        CUdeviceptr host = (CUdeviceptr)hostPtr;
        //If the hostPtr is already pinned
        if(CudaDriver::pinnedHostMemCounter.find(host) != CudaDriver::pinnedHostMemCounter.end() && CudaDriver::pinnedHostMemCounter[host] > 0)	{
            CudaDriver::pinnedHostMemCounter[host]++;
        } else {
            CUresult cuResult = cuMemHostRegister((void*)host, size, CU_MEMHOSTREGISTER_PORTABLE);
            if(cuResult == CUDA_SUCCESS) {
                CudaDriver::pinnedHostMemCounter[host] = 1;
#ifdef _OPENARC_PROFILE_
    			HostConf_t * tconf = getHostConf(threadID);
				tconf->IPMallocCnt++;
				tconf->IPMallocSize += size;
#endif
            } else	{
				result = HI_error;
            }

        }
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_pin_host_memory()\n");
	}
#endif
	return result;
}

// Pin host memory if unpinned.
// If the memory is already pinned, it does not increase pinnedHostMemCounter.
// [CAUTION] this will work only if hostPtr refers to the base address of allocated
// memory.
HI_error_t CudaDriver::pin_host_memory_if_unpinned(const void* hostPtr, size_t size, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::pin_host_memory_if_unpinned()\n");
	}
#endif
    HI_error_t result = HI_success;
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
        CUdeviceptr host = (CUdeviceptr)hostPtr;
        //If the hostPtr is already pinned
        if(CudaDriver::pinnedHostMemCounter.find(host) == CudaDriver::pinnedHostMemCounter.end() )	{
            CUresult cuResult = cuMemHostRegister((void*)host, size, CU_MEMHOSTREGISTER_PORTABLE);
            if(cuResult == CUDA_SUCCESS) {
                CudaDriver::pinnedHostMemCounter[host] = 1;
#ifdef _OPENARC_PROFILE_
    			HostConf_t * tconf = getHostConf(threadID);
				tconf->IPMallocCnt++;
				tconf->IPMallocSize += size;
#endif
            } else	{
				result = HI_error;
            }
        }
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::pin_host_memory_if_unpinned()\n");
	}
#endif
	return result;
}

void CudaDriver::HI_unpin_host_memory(const void* hostPtr, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_unpin_host_memory()\n");
	}
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
        CUdeviceptr host = (CUdeviceptr)hostPtr;
        //Unpin if the hostPtr is already pinned
        if(CudaDriver::pinnedHostMemCounter.find(host) != CudaDriver::pinnedHostMemCounter.end()) {
            if(CudaDriver::pinnedHostMemCounter[host] > 1) {
                CudaDriver::pinnedHostMemCounter[host]--;
            } else
            {
                CUresult cuResult = cuMemHostUnregister((void*)host);
                if(cuResult == CUDA_SUCCESS){
                	//CudaDriver::pinnedHostMemCounter[host] = 0;
                	CudaDriver::pinnedHostMemCounter.erase(host);
#ifdef _OPENARC_PROFILE_
					HostConf_t * tconf = getHostConf(threadID);
					tconf->IPFreeCnt++;
#endif
                } else {
                	fprintf(stderr, "[ERROR in CudaDriver::HI_unpin_host_memory()] Cannot unpin host memory with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
					exit(1);
                }
            }
        }
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_unpin_host_memory()\n");
	}
#endif
}

void CudaDriver::dec_pinned_host_memory_counter(const void* hostPtr)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::dec_pinned_host_memory_counter()\n");
	}
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
        CUdeviceptr host = (CUdeviceptr)hostPtr;
        //If the hostPtr is already pinned
        if(CudaDriver::pinnedHostMemCounter.find(host) != CudaDriver::pinnedHostMemCounter.end()) {
			CudaDriver::pinnedHostMemCounter[host]--;
        } else {
			fprintf(stderr, "[ERROR in CudaDriver::dec_pinned_host_memory_counter()] \n");
		}
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::dec_pinned_host_memory_counter()\n");
	}
#endif
}

void CudaDriver::inc_pinned_host_memory_counter(const void* hostPtr)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::inc_pinned_host_memory_counter()\n");
	}
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
        CUdeviceptr host = (CUdeviceptr)hostPtr;
        //If the hostPtr is already pinned
        if(CudaDriver::pinnedHostMemCounter.find(host) != CudaDriver::pinnedHostMemCounter.end()) {
			CudaDriver::pinnedHostMemCounter[host]++;
        } else {
			fprintf(stderr, "[ERROR in CudaDriver::inc_pinned_host_memory_counter()] \n");
			exit(1);
		}
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::inc_pinned_host_memory_counter()\n");
	}
#endif
}

//Unpin host memories whose counters are less than 1.
//This also frees corresponding device memory.
void CudaDriver::unpin_host_memory_all(int asyncID, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::unpin_host_memory_all(%d)\n", asyncID);
	}
    HostConf_t * tconf = getHostConf(threadID);
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_victim_cache);
#else
#ifdef _OPENMP
    	#pragma omp critical (victim_cache_critical)
#endif
#endif
		{
		addresstable_t::iterator it = CudaDriver::auxAddressTable.find(asyncID);
		if(it != CudaDriver::auxAddressTable.end()) {
			CudaDriver::hostMemToUnpin.clear();
			for( addressmap_t::iterator it2 = (it->second)->begin(); it2 != (it->second)->end(); ++it2 ) {
        		CUdeviceptr host = (CUdeviceptr)it2->first;
        		//If the hostPtr is already pinned
        		if(CudaDriver::pinnedHostMemCounter.find(host) != CudaDriver::pinnedHostMemCounter.end()) {
            		if(CudaDriver::pinnedHostMemCounter[host] < 1) {
                		CUresult cuResult = cuMemHostUnregister((void*)host);
                		if(cuResult == CUDA_SUCCESS){
                			CudaDriver::pinnedHostMemCounter.erase(host);
							CudaDriver::hostMemToUnpin.push_back(it2->first);
#ifdef _OPENARC_PROFILE_
							tconf->IPFreeCnt++;
#endif
                		} else {
                			fprintf(stderr, "[ERROR in CudaDriver::unpin_host_memory_all(%d)] Cannot unpin host memory with error %d (%s)\n", asyncID, cuResult, cuda_error_code(cuResult));
							exit(1);
                		}
						//Free corresponding device memory.
						addresstable_entity_t *aet = (addresstable_entity_t*) it2->second;
						cuMemFree((CUdeviceptr)aet->basePtr);
#ifdef _OPENARC_PROFILE_
						tconf->IDFreeCnt++;
						//[FIXME] tconf->CIDMemorySize should be adjusted here.
#endif
				
            		}
        		}
			}
			while( !CudaDriver::hostMemToUnpin.empty() ) {
				(it->second)->erase(CudaDriver::hostMemToUnpin.back());
				CudaDriver::hostMemToUnpin.pop_back();
			}
		}
		}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_victim_cache);
#endif
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::unpin_host_memory_all(%d)\n", asyncID);
	}
#endif
}

//Unpin host memories whose counters are less than 1.
void CudaDriver::unpin_host_memory_all(int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::unpin_host_memory_all()\n");
	}
    HostConf_t * tconf = getHostConf(threadID);
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_pin_host_memory);
#else
#ifdef _OPENMP
    #pragma omp critical (pin_host_memory_critical)
#endif
#endif
    {
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_victim_cache);
#else
#ifdef _OPENMP
    	#pragma omp critical (victim_cache_critical)
#endif
#endif
		for( addresstable_t::iterator it = CudaDriver::auxAddressTable.begin(); it != CudaDriver::auxAddressTable.end(); ++it) {
			CudaDriver::hostMemToUnpin.clear();
			for( addressmap_t::iterator it2 = (it->second)->begin(); it2 != (it->second)->end(); ++it2 ) {
        		CUdeviceptr host = (CUdeviceptr)it2->first;
        		//If the hostPtr is already pinned
        		if(CudaDriver::pinnedHostMemCounter.find(host) != CudaDriver::pinnedHostMemCounter.end()) {
            		if(CudaDriver::pinnedHostMemCounter[host] < 1) {
                		CUresult cuResult = cuMemHostUnregister((void*)host);
                		if(cuResult == CUDA_SUCCESS){
                			CudaDriver::pinnedHostMemCounter.erase(host);
							CudaDriver::hostMemToUnpin.push_back(it2->first);
#ifdef _OPENARC_PROFILE_
							tconf->IDFreeCnt++;
							//[FIXME] tconf->CIDMemorySize should be adjusted here.
#endif
                		} else {
                			fprintf(stderr, "[ERROR in CudaDriver::unpin_host_memory_all()] Cannot unpin host memory with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
							exit(1);
                		}
						//Free corresponding device memory.
						addresstable_entity_t *aet = (addresstable_entity_t*) it2->second;
						cuMemFree((CUdeviceptr)aet->basePtr);
#ifdef _OPENARC_PROFILE_
						tconf->IDFreeCnt++;
						//[FIXME] tconf->CIDMemorySize should be adjusted here.
#endif
            		}
        		}
			}
			while( !CudaDriver::hostMemToUnpin.empty() ) {
				(it->second)->erase(CudaDriver::hostMemToUnpin.back());
				CudaDriver::hostMemToUnpin.pop_back();
			}
		}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_victim_cache);
#endif
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_pin_host_memory);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::unpin_host_memory_all()\n");
	}
#endif
}

void CudaDriver::release_freed_device_memory(int asyncID, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::release_freed_device_memory(%d)\n", asyncID);
	}
    HostConf_t * tconf = getHostConf(threadID);
#endif
    {
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_victim_cache);
#else
#ifdef _OPENMP
    	#pragma omp critical (victim_cache_critical)
#endif
#endif
		{
		addresstable_t::iterator it = CudaDriver::auxAddressTable.find(asyncID);
		if(it != CudaDriver::auxAddressTable.end()) {
			for( addressmap_t::iterator it2 = (it->second)->begin(); it2 != (it->second)->end(); ++it2 ) {
				//Free corresponding device memory.
				addresstable_entity_t *aet = (addresstable_entity_t*) it2->second;
				cuMemFree((CUdeviceptr)aet->basePtr);
#ifdef _OPENARC_PROFILE_
				tconf->IDFreeCnt++;
				//[FIXME] tconf->CIDMemorySize should be adjusted here.
#endif
			}
		}
		}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_victim_cache);
#endif
    }
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::release_freed_device_memory(%d)\n", asyncID);
	}
#endif
}

void CudaDriver::release_freed_device_memory(int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::release_freed_device_memory()\n");
	}
    HostConf_t * tconf = getHostConf(threadID);
#endif
    {
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_victim_cache);
#else
#ifdef _OPENMP
    	#pragma omp critical (victim_cache_critical)
#endif
#endif
		for( addresstable_t::iterator it = CudaDriver::auxAddressTable.begin(); it != CudaDriver::auxAddressTable.end(); ++it ) {
			for( addressmap_t::iterator it2 = (it->second)->begin(); it2 != (it->second)->end(); ++it2 ) {
				//Free corresponding device memory.
				addresstable_entity_t *aet = (addresstable_entity_t*) it2->second;
				cuMemFree((CUdeviceptr)aet->basePtr);
#ifdef _OPENARC_PROFILE_
				tconf->IDFreeCnt++;
				//[FIXME] tconf->CIDMemorySize should be adjusted here.
#endif
			}
		}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_victim_cache);
#endif
    }
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::release_freed_device_memory()\n");
	}
#endif
}

HI_error_t  CudaDriver::HI_malloc1D(const void *hostPtr, void **devPtr, size_t count, int asyncID, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_malloc1D(hostPtr = %lx, asyncID = %d, size = %lu, thread ID = %d)\n",(long unsigned int)hostPtr, asyncID, count, threadID);
	}
#endif
#ifdef INIT_DEBUG
    fprintf(stderr, "[DEBUG] call HI_malloc1D().\n");
	fprintf(stderr, "[DEBUG] Current host thread: %ld\n", syscall(__NR_gettid));
#endif
    HostConf_t * tconf = getHostConf(threadID);
    if( tconf == NULL ) {
        //fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] No host configuration exists for the current host thread (thread ID: %d); please set an environment variable, OMP_NUM_THREADS, to the maximum number of OpenMP threads used for your application; exit!\n", tconf->threadID);
        fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] No host configuration exists for the current host thread (thread ID: %d); exit!\n", threadID);
        exit(1);
    }
    if( tconf->device->init_done == 0 ) {
        tconf->HI_init(DEVICE_NUM_UNDEFINED);
    }
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    HI_error_t result = HI_error;

    if(HI_get_device_address(hostPtr, devPtr, NULL, NULL, asyncID, tconf->threadID) == HI_success ) {
		if( unifiedMemSupported ) {
        	result = HI_success;
        } else {
			fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] Duplicate device memory allocation for the same host data (%lx) by thread %d is not allowed; exit!\n",(long unsigned int)hostPtr, tconf->threadID);
			exit(1);
		}
    } else {
        CUresult cuResult = CUDA_SUCCESS;
#if VICTIM_CACHE_MODE <= 1
		memPool_t *memPool = memPoolMap[tconf->threadID];
        std::multimap<size_t, void *>::iterator it = memPool->find(count);
        if (it != memPool->end()) {
#ifdef _OPENARC_PROFILE_
			if( HI_openarcrt_verbosity > 2 ) {
				fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_malloc1D(%d, %lu) reuses memories in the memPool\n", asyncID, count);
			}
#endif
			*devPtr = it->second;
            memPool->erase(it);
			current_mempool_size -= count;
        } else {
			if( current_mempool_size > tconf->max_mempool_size ) {
#ifdef _OPENARC_PROFILE_
				if( HI_openarcrt_verbosity > 2 ) {
					fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_malloc1D(%d, %lu) releases memories in the memPool\n", asyncID, count);
				}
#endif
                for (it = memPool->begin(); it != memPool->end(); ++it) {
            		*devPtr = it->second;
					cuResult = cuMemFree((CUdeviceptr)(*devPtr));
					current_mempool_size -= it->first;
#ifdef _OPENARC_PROFILE_
            		tconf->IDFreeCnt++;
					tconf->CIDMemorySize -= it->first;
#endif
                    if(cuResult != CUDA_SUCCESS) {
                        fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
                    }
                }
                memPool->clear();
			}
            cuResult = cuMemAlloc((CUdeviceptr*)devPtr, count);
#ifdef _OPENARC_PROFILE_
            tconf->IDMallocCnt++;
            tconf->IDMallocSize += count;
            tconf->CIDMemorySize += count;
			if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
				tconf->MIDMemorySize = tconf->CIDMemorySize;
			}
#endif
            if (cuResult != CUDA_SUCCESS) {
#ifdef _OPENARC_PROFILE_
				if( HI_openarcrt_verbosity > 2 ) {
					fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_malloc1D(%d, %lu) releases memories in the memPool\n", asyncID, count);
				}
#endif
                for (it = memPool->begin(); it != memPool->end(); ++it) {
            		*devPtr = it->second;
					cuResult = cuMemFree((CUdeviceptr)(*devPtr));
					current_mempool_size -= it->first;
#ifdef _OPENARC_PROFILE_
            		tconf->IDFreeCnt++;
					tconf->CIDMemorySize -= it->first;
#endif
                    if(cuResult != CUDA_SUCCESS) {
                        fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
                    }
                }
                memPool->clear();
                cuResult = cuMemAlloc((CUdeviceptr*)devPtr, count);
            }
        }
        if( cuResult == CUDA_SUCCESS ) {
          	//Pin host memory
            if( tconf->prepin_host_memory == 1 ) {
            	if( HI_pin_host_memory(hostPtr, (size_t) count, tconf->threadID) == HI_error ) {
#ifdef _OPENMP
                	fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", omp_get_thread_num());
					exit(1);
#else
                	fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", tconf->threadID);
					exit(1);
#endif
				}
			}
            HI_set_device_address(hostPtr, *devPtr, (size_t) count, asyncID, tconf->threadID);
            result = HI_success;
        } else {
            fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] CUDA memory alloc failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
			exit(1);
        }
#else
		///////////////////////////
		// VICTIM_CACHE_MODE = 2 //
		///////////////////////////
    	if(HI_get_device_address_from_victim_cache(hostPtr, devPtr, NULL, NULL, asyncID, tconf->threadID) == HI_success ) {
			result = HI_success;
            if( tconf->prepin_host_memory == 1 ) {
				inc_pinned_host_memory_counter(hostPtr);
			}
			HI_remove_device_address_from_victim_cache(hostPtr, asyncID, tconf->threadID);
        } else {
#ifdef _OPENARC_PROFILE_
            tconf->IDMallocCnt++;
            tconf->IDMallocSize += count;
            tconf->CIDMemorySize += count;
			if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
				tconf->MIDMemorySize = tconf->CIDMemorySize;
			}
#endif
            cuResult = cuMemAlloc((CUdeviceptr*)devPtr, (size_t) count);
            if (cuResult != CUDA_SUCCESS) {
            	if( tconf->prepin_host_memory == 1 ) {
					unpin_host_memory_all(asyncID);
				} else {
					release_freed_device_memory(asyncID, tconf->threadID);
					HI_reset_victim_cache(asyncID);
				}
				//Try to allocate device memory again.
            	cuResult = cuMemAlloc((CUdeviceptr*)devPtr, (size_t) count);
            	if (cuResult != CUDA_SUCCESS) {
            		if( tconf->prepin_host_memory == 1 ) {
						unpin_host_memory_all();
					} else {
						release_freed_device_memory(tconf->threadID);
						HI_reset_victim_cache_all();
					}
					//Try to allocate device memory again.
            		cuResult = cuMemAlloc((CUdeviceptr*)devPtr, (size_t) count);
				}
			}
        	if( cuResult == CUDA_SUCCESS ) {
          		//Pin host memory
            	if( tconf->prepin_host_memory == 1 ) {
            		result = HI_pin_host_memory(hostPtr, (size_t) count, tconf->threadID);
					if( result != HI_success ) {
						unpin_host_memory_all(asyncID);
            			result = HI_pin_host_memory(hostPtr, (size_t) count, tconf->threadID);
					}
					if( result != HI_success ) {
						unpin_host_memory_all();
            			result = HI_pin_host_memory(hostPtr, (size_t) count, tconf->threadID);
					}
					if( result != HI_success ) {
#ifdef _OPENMP
                		fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", omp_get_thread_num());
						exit(1);
#else
						fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", tconf->threadID);
						exit(1);
#endif
					}
				}
			}
		}
        if( cuResult == CUDA_SUCCESS ) {
            HI_set_device_address(hostPtr, *devPtr, (size_t) count, asyncID, tconf->threadID);
            result = HI_success;
        } else {
            fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D()] CUDA memory alloc failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
			exit(1);
        }
#endif
    }

#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		HI_print_device_address_mapping_summary(tconf->threadID);
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_malloc1D(hostPtr = %lx, asyncID = %d, size = %lu, thread ID = %d)\n",(long unsigned int)hostPtr, asyncID, count, threadID);
	}
#endif
    return result;
}

HI_error_t  CudaDriver::HI_malloc1D_unified(const void *hostPtr, void **devPtr, size_t count, int asyncID, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_malloc1D_unified(%d, %lu)\n", asyncID, count);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    if( tconf == NULL ) {
#ifdef _OPENMP
        int thread_id = omp_get_thread_num();
#else
        int thread_id = 0;
#endif
        fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D_unified()] No host configuration exists for the current host thread (thread ID: %d); please set an environment variable, OMP_NUM_THREADS, to the maximum number of OpenMP threads used for your application; exit!\n", thread_id);
        exit(1);
    }
    if( tconf->device->init_done == 0 ) {
        tconf->HI_init(DEVICE_NUM_UNDEFINED);
    }
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    HI_error_t result = HI_error;


    if(HI_get_device_address(hostPtr, devPtr, NULL, NULL, asyncID, tconf->threadID) == HI_success ) {
        //result = HI_success;
		fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D_unified()] Duplicate device memory allocation for the same host data by thread %d is not allowed; exit!\n", tconf->threadID);
		exit(1);
    } else {
#if CUDA_VERSION >= 6000
		if( unifiedMemSupported == 0 ) {
			fprintf(stderr, "[OPENARCRT-WARNING in CudaDriver::HI_malloc1D_unified(%d)] unified memory is either not supported or disabled in the current device; device memory should be explicitly managed either through data clauses or though runtime APIs.\n(To enable unified memory, environment variable, OPENARCRT_UNIFIEDMEM should be set to 1.)\n", asyncID);
			if( hostPtr == NULL ) {
				*devPtr = malloc(count);
			} else {
				*devPtr = (void *)hostPtr;
			}
#ifdef _OPENARC_PROFILE_
            tconf->IDMallocCnt++;
            tconf->IDMallocSize += count;
            tconf->CIDMemorySize += count;
			if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
				tconf->MIDMemorySize = tconf->CIDMemorySize;
			}
#endif
		} else {
			//CU_MEM_ATTACH_GLOBAL: this memory is accessible from any stream on any device.
			//CU_MEM_ATTACH_HOST: the allocation is created within initial visibility restricted to host access
			//only; an explicit call to cuStreamAttachMemAsync will be required to enable access on the device.
        	CUresult cuResult = cuMemAllocManaged((CUdeviceptr*)devPtr, count, CU_MEM_ATTACH_GLOBAL);
#ifdef _OPENARC_PROFILE_
            tconf->IDMallocCnt++;
            tconf->IDMallocSize += count;
            tconf->CIDMemorySize += count;
			if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
				tconf->MIDMemorySize = tconf->CIDMemorySize;
			}
#endif
        	if( cuResult == CUDA_SUCCESS ) {
            	HI_set_device_address(*devPtr, *devPtr, count, asyncID, tconf->threadID);
            	result = HI_success;
        	} else {
            	fprintf(stderr, "[ERROR in CudaDriver::HI_malloc1D_unified()] CUDA memory alloc failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
        	}
		}
#else
		fprintf(stderr, "[OPENARCRT-WARNING in CudaDriver::HI_malloc1D_unified(%d)] To use the unified memory, CUDA toolkit version 6 or later should be used (currrent version = %d); device memory should be explicitly managed either through data clauses or though runtime APIs.\n", CUDA_VERSION, asyncID);
		if( hostPtr == NULL ) {
			*devPtr = malloc(count);
		} else {
			*devPtr = (void *)hostPtr;
		}
#endif
    }

#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_malloc1D_unified(%d, %lu)\n", asyncID, count);
	}
#endif
    return result;
}

//the ElementSizeBytes in cuMemAllocPitch is currently set to 16.
HI_error_t CudaDriver::HI_malloc2D( const void *hostPtr, void** devPtr, size_t* pitch, size_t widthInBytes, size_t height, int asyncID, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_malloc2D(%d)\n", asyncID);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

    if( tconf->device->init_done == 0 ) {
        tconf->HI_init(DEVICE_NUM_UNDEFINED);
    }
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    HI_error_t result;

    if(HI_get_device_address(hostPtr, devPtr, NULL, NULL, asyncID, tconf->threadID) == HI_success ) {
		if( unifiedMemSupported ) {
        	result = HI_success;
        } else {
			fprintf(stderr, "[ERROR in CudaDriver::HI_malloc2D()] Duplicate device memory allocation for the same host data by thread %d is not allowed; exit!\n", tconf->threadID);
			exit(1);
		}
    } else {
#ifdef _OPENARC_PROFILE_
		tconf->IDMallocCnt++;
		tconf->IDMallocSize += widthInBytes*height;
        tconf->CIDMemorySize += widthInBytes*height;
		if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
			tconf->MIDMemorySize = tconf->CIDMemorySize;
		}
#endif
        CUresult cuResult = cuMemAllocPitch((CUdeviceptr*)devPtr, pitch, widthInBytes, height, 16);
        if( cuResult == CUDA_SUCCESS ) {
            //Pin host memory
            if( tconf->prepin_host_memory == 1 ) {
            	if( HI_pin_host_memory(hostPtr, (size_t) widthInBytes*height, tconf->threadID) == HI_error ) {
#ifdef _OPENMP
                	fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", omp_get_thread_num());
					exit(1);
#else
                	fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", tconf->threadID);
					exit(1);
#endif
				}
			}

            HI_set_device_address(hostPtr, *devPtr, (size_t) widthInBytes*height, asyncID, tconf->threadID);
            result = HI_success;
        } else {
            fprintf(stderr, "[ERROR in CudaDriver::HI_malloc2D()] CUDA memory alloc failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
			exit(1);
        }
    }

#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_malloc2D(%d)\n", asyncID);
	}
#endif
    return result;
}


HI_error_t CudaDriver::HI_malloc3D( const void *hostPtr, void** devPtr, size_t* pitch, size_t widthInBytes, size_t height, size_t depth, int asyncID, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_malloc3D(%d)\n", asyncID);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    //TODO
    HI_error_t result;
    result = HI_error;
#ifdef _OPENARC_PROFILE_
    tconf->IDMallocCnt++;
    tconf->IDMallocSize += widthInBytes*height*depth;
    tconf->CIDMemorySize += widthInBytes*height*depth;
	if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
		tconf->MIDMemorySize = tconf->CIDMemorySize;
	}
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_malloc3D(%d)\n", asyncID);
	}
#endif
    return result;
}



HI_error_t CudaDriver::HI_free( const void *hostPtr, int asyncID, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_free(%d)\n", asyncID);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif

    HI_error_t result = HI_success;
    void *devPtr;
    size_t size;
    //Check if the mapping exists. Free only if a mapping is found
    if( HI_get_device_address(hostPtr, &devPtr, NULL, &size, asyncID, tconf->threadID) != HI_error) {
		//If this method is called for unified memory, memory deallocation
		//is skipped; unified memory will be deallocatedd only by 
		//HI_free_unified().
		if( hostPtr != devPtr ) {
#if VICTIM_CACHE_MODE == 1
			//We do not free the device memory; instead put it in the memory pool 
			//and remove host-pointer-to-device-pointer mapping.
			memPool_t *memPool = memPoolMap[tconf->threadID];
            memPool->insert(std::pair<size_t, void *>(size, devPtr));
			current_mempool_size += size;
			HI_remove_device_address(hostPtr, asyncID, tconf->threadID);
			// Unpin host memory
			HI_unpin_host_memory(hostPtr, tconf->threadID);
#endif
#if VICTIM_CACHE_MODE == 2
			///////////////////////////
			// VICTIM_CACHE_MODE = 2 //
			///////////////////////////
			HI_remove_device_address(hostPtr, asyncID, tconf->threadID);
			HI_set_device_address_in_victim_cache(hostPtr, devPtr, size, asyncID, tconf->threadID);
           // Decrease pinned host memory counter
            if( tconf->prepin_host_memory == 1 ) {
            	dec_pinned_host_memory_counter(hostPtr);
			} else {
				HI_unpin_host_memory(hostPtr, tconf->threadID);
			}
#endif
#if VICTIM_CACHE_MODE == 0
            CUresult cuResult = CUDA_SUCCESS;
			cuResult = cuMemFree((CUdeviceptr)(devPtr));
        	if( cuResult == CUDA_SUCCESS ) {
            	HI_remove_device_address(hostPtr, asyncID, tconf->threadID);
            	// Unpin host memory
            	if( tconf->prepin_host_memory == 1 ) {
            		HI_unpin_host_memory(hostPtr, tconf->threadID);
				}
        	} else {
            	fprintf(stderr, "[ERROR in CudaDriver::HI_free()] CUDA memory free failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
            	result = HI_error;
        	}
#ifdef _OPENARC_PROFILE_
            tconf->IDFreeCnt++;
        	tconf->CIDMemorySize -= size;
#endif
#endif
		}
    }
#ifdef _OPENARC_PROFILE_
    tconf->totalFreeTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_free(%d)\n", asyncID);
	}
#endif
    return result;
}

HI_error_t CudaDriver::HI_free_unified( const void *hostPtr, int asyncID, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_free_unified(%d)\n", asyncID);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif

    HI_error_t result = HI_success;
    void *devPtr;
	size_t memSize = 0;
    //Check if the mapping exists. Free only if a mapping is found
    if( HI_get_device_address(hostPtr, &devPtr, NULL, &memSize, asyncID, tconf->threadID) == HI_success) {
		if( unifiedMemSupported == 0 ) {
			free(devPtr);
#ifdef _OPENARC_PROFILE_
            	tconf->IDFreeCnt++;
        		tconf->CIDMemorySize -= memSize;
#endif
		} else {
        	CUresult cuResult = cuMemFree((CUdeviceptr)(devPtr));
        	if( cuResult == CUDA_SUCCESS ) {
            	HI_remove_device_address(hostPtr, asyncID, tconf->threadID);

#ifdef _OPENARC_PROFILE_
            	tconf->IDFreeCnt++;
        		tconf->CIDMemorySize -= memSize;
#endif
        	} else {
            	fprintf(stderr, "[ERROR in CudaDriver::HI_free_unified()] CUDA memory free failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
            	result = HI_error;
        	}
		}
    }
#ifdef _OPENARC_PROFILE_
	tconf->totalFreeTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_free_unified(%d)\n", asyncID);
	}
#endif
    return result;
}


#if VICTIM_CACHE_MODE == 0

//malloc used for allocating temporary data.
//If the method is called for a pointer to existing memory, the existing memory
//will be freed before allocating new memory.
void CudaDriver::HI_tempMalloc1D( void** tempPtr, size_t count, acc_device_t devType, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_tempMalloc1D()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_tempMalloc);
#else
#ifdef _OPENMP
    #pragma omp critical (tempMalloc_critical)
#endif
#endif
	{
    if( devType == acc_device_gpu || devType == acc_device_nvidia || 
		devType == acc_device_radeon || devType == acc_device_current) {
		if( tempMallocSet.count(*tempPtr) > 0 ) {
			tempMallocSet.erase(*tempPtr);	
    		CUresult cuResult = cuMemFree((CUdeviceptr)*tempPtr);
    		if(cuResult != CUDA_SUCCESS) {
        		fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
    		}
#ifdef _OPENARC_PROFILE_
            tconf->IDFreeCnt++;
#endif
			if( tempMallocSizeMap.count(tconf->threadID) > 0 ) {
				sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
				if( tempMallocSize->count(*tempPtr) > 0 ) {
#ifdef _OPENARC_PROFILE_
					tconf->CIDMemorySize -= (*tempMallocSize)[*tempPtr];
#endif
					tempMallocSize->erase((const void *)*tempPtr);
				}
			}
		}
    	CUresult cuResult = cuMemAlloc((CUdeviceptr*)tempPtr, (size_t) count);
    	if(cuResult != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D()] failed to malloc on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
			exit(1);
    	}
		tempMallocSet.insert(*tempPtr);	
		sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
		(*tempMallocSize)[(const void *)*tempPtr] = count;
#ifdef _OPENARC_PROFILE_
        tconf->IDMallocCnt++;
        tconf->IDMallocSize += count;
        tconf->CIDMemorySize += count;
		if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
			tconf->MIDMemorySize = tconf->CIDMemorySize;
		}
#endif
    } else {
		if( tempMallocSet.count(*tempPtr) > 0 ) {
			tempMallocSet.erase(*tempPtr);	
            free(*tempPtr);
#ifdef _OPENARC_PROFILE_
            tconf->IHFreeCnt++;
#endif
        }
        *tempPtr = malloc(count);
		tempMallocSet.insert(*tempPtr);	
#ifdef _OPENARC_PROFILE_
        tconf->IHMallocCnt++;
        tconf->IHMallocSize += count;
        tconf->CIHMemorySize += count;
		if( tconf->MIHMemorySize < tconf->CIHMemorySize ) {
			tconf->MIHMemorySize = tconf->CIHMemorySize;
		}
#endif
    }
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_tempMalloc);
#endif
#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_tempMalloc1D()\n");
	}
#endif
}

//malloc used for allocating temporary data.
void CudaDriver::HI_tempMalloc1D_async( void** tempPtr, size_t count, acc_device_t devType, HI_MallocKind_t flags, int asyncID, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_tempMalloc1D_async()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_tempMalloc);
#else
#ifdef _OPENMP
    #pragma omp critical (tempMalloc_critical)
#endif
#endif
	{
    if( devType == acc_device_gpu || devType == acc_device_nvidia || 
		devType == acc_device_radeon || devType == acc_device_current) {
    	CUresult cuResult = cuMemAlloc((CUdeviceptr*)tempPtr, (size_t) count);
    	if(cuResult != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D_async()] failed to malloc on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
			exit(1);
    	}
		tempMallocSet.insert(*tempPtr);	
		sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
		(*tempMallocSize)[(const void *)*tempPtr] = count;
#ifdef _OPENARC_PROFILE_
        tconf->IDMallocCnt++;
        tconf->IDMallocSize += count;
        tconf->CIDMemorySize += count;
		if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
			tconf->MIDMemorySize = tconf->CIDMemorySize;
		}
#endif
    } else {
        *tempPtr = malloc(count);
		tempMallocSet.insert(*tempPtr);	
#ifdef _OPENARC_PROFILE_
        tconf->IHMallocCnt++;
        tconf->IHMallocSize += count;
        tconf->CIHMemorySize += count;
		if( tconf->MIHMemorySize < tconf->CIHMemorySize ) {
			tconf->MIHMemorySize = tconf->CIHMemorySize;
		}
#endif
    }
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_tempMalloc);
#endif
#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_tempMalloc1D_async()\n");
	}
#endif
}

//Used for de-allocating temporary data.
void CudaDriver::HI_tempFree( void** tempPtr, acc_device_t devType, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_tempFree(tempPtr = %lx, devType = %d, thread ID = %d)\n",  (long unsigned int)(*tempPtr), devType, threadID);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_tempMalloc);
#else
#ifdef _OPENMP
    #pragma omp critical (tempMalloc_critical)
#endif
#endif
	{
    if( devType == acc_device_gpu || devType == acc_device_nvidia 
    || devType == acc_device_radeon || devType == acc_device_current ) {
        if( *tempPtr != 0 ) {
			tempMallocSet.erase(*tempPtr);	
            //cudaFree(*tempPtr);
    		CUresult cuResult = cuMemFree((CUdeviceptr)*tempPtr);
    		if(cuResult != CUDA_SUCCESS) {
        		fprintf(stderr, "[ERROR in CudaDriver::HI_tempFree()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
    		}
#ifdef _OPENARC_PROFILE_
            tconf->IDFreeCnt++;
#endif
			if( tempMallocSizeMap.count(tconf->threadID) > 0 ) {
				sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
				if( tempMallocSize->count(*tempPtr) > 0 ) {
#ifdef _OPENARC_PROFILE_
					tconf->CIDMemorySize -= (*tempMallocSize)[*tempPtr];
#endif
					tempMallocSize->erase((const void *)*tempPtr);
				}
			}
        }
    } else {
        if( *tempPtr != 0 ) {
			tempMallocSet.erase(*tempPtr);	
            free(*tempPtr);
            if( tconf->prepin_host_memory == 1 ) {
    			// Unpin host memory if already pinned.
    			HI_unpin_host_memory(*tempPtr, tconf->threadID);
			}
#ifdef _OPENARC_PROFILE_
            tconf->IHFreeCnt++;
#endif
        }
    }
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_tempMalloc);
#endif
    *tempPtr = 0;
#ifdef _OPENARC_PROFILE_
    tconf->totalFreeTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_tempFree(tempPtr = %lx, devType = %d, thread ID = %d)\n",  (long unsigned int)(*tempPtr), devType, threadID);
	}
#endif
}

#else

///////////////////////////
// VICTIM_CACHE_MODE > 0 //
///////////////////////////

//malloc used for allocating temporary data.
void CudaDriver::HI_tempMalloc1D( void** tempPtr, size_t count, acc_device_t devType, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_tempMalloc1D()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_tempMalloc);
#else
#ifdef _OPENMP
    #pragma omp critical (tempMalloc_critical)
#endif
#endif
	{
    if( devType == acc_device_gpu || devType == acc_device_nvidia || 
		devType == acc_device_radeon || devType == acc_device_current) {
    	CUresult cuResult = CUDA_SUCCESS;
        memPool_t *memPool = memPoolMap[tconf->threadID];
		sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
        std::multimap<size_t, void *>::iterator it = memPool->find(count);
        if (it != memPool->end()) {
#ifdef _OPENARC_PROFILE_
            if( HI_openarcrt_verbosity > 2 ) {
                fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_tempMalloc1D(%lu) reuses memories in the memPool\n", count);
            }
#endif           
            *tempPtr = it->second;
            memPool->erase(it);
			current_mempool_size -= count;
			(*tempMallocSize)[(const void *)*tempPtr] = count;
        } else { 
			if( current_mempool_size > tconf->max_mempool_size ) {
#ifdef _OPENARC_PROFILE_
                if( HI_openarcrt_verbosity > 2 ) {
                    fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_malloc1D(%lu) releases memories in the memPool\n", count);
                }
#endif               
                for (it = memPool->begin(); it != memPool->end(); ++it) {
                    *tempPtr = it->second;
                    cuResult = cuMemFree((CUdeviceptr)(*tempPtr));
					current_mempool_size -= it->first;
#ifdef _OPENARC_PROFILE_ 
                    tconf->IDFreeCnt++;
        			tconf->CIDMemorySize -= it->first;
#endif                   
                    if(cuResult != CUDA_SUCCESS) {
                        fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
                    }
                }        
                memPool->clear();
			}
            cuResult = cuMemAlloc((CUdeviceptr*)tempPtr, count);
            if (cuResult != CUDA_SUCCESS) {
#ifdef _OPENARC_PROFILE_
                if( HI_openarcrt_verbosity > 2 ) {
                    fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_malloc1D(%lu) releases memories in the memPool\n", count);
                }
#endif               
                for (it = memPool->begin(); it != memPool->end(); ++it) {
                    *tempPtr = it->second;
                    cuResult = cuMemFree((CUdeviceptr)(*tempPtr));
					current_mempool_size -= it->first;
#ifdef _OPENARC_PROFILE_ 
                    tconf->IDFreeCnt++;
        			tconf->CIDMemorySize -= it->first;
#endif                   
                    if(cuResult != CUDA_SUCCESS) {
                        fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
                    }
                }        
                memPool->clear();
                cuResult = cuMemAlloc((CUdeviceptr*)tempPtr, count);
            }        
    		if(cuResult == CUDA_SUCCESS) {
				//New temporary device memory is allocated.
				(*tempMallocSize)[(const void *)*tempPtr] = count;
#ifdef _OPENARC_PROFILE_
		        tconf->IDMallocCnt++;
        		tconf->IDMallocSize += count;
        		tconf->CIDMemorySize += count;
				if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
					tconf->MIDMemorySize = tconf->CIDMemorySize;
				}
#endif
			} else {
        		fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D()] failed to malloc on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
    		}
        }        
    } else {
		if( tempMallocSet.count(*tempPtr) > 0 ) {
			tempMallocSet.erase(*tempPtr);	
            free(*tempPtr);
#ifdef _OPENARC_PROFILE_
            tconf->IHFreeCnt++;
#endif
        }
        *tempPtr = malloc(count);
		tempMallocSet.insert(*tempPtr);	
#ifdef _OPENARC_PROFILE_
        tconf->IHMallocCnt++;
        tconf->IHMallocSize += count;
        tconf->CIHMemorySize += count;
		if( tconf->MIHMemorySize < tconf->CIHMemorySize ) {
			tconf->MIHMemorySize = tconf->CIHMemorySize;
		}
#endif
    }
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_tempMalloc);
#endif
#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_tempMalloc1D()\n");
	}
#endif
}

//malloc used for allocating temporary data.
void CudaDriver::HI_tempMalloc1D_async( void** tempPtr, size_t count, acc_device_t devType, HI_MallocKind_t flags, int asyncID, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_tempMalloc1D_async()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_tempMalloc);
#else
#ifdef _OPENMP
    #pragma omp critical (tempMalloc_critical)
#endif
#endif
	{
    if( devType == acc_device_gpu || devType == acc_device_nvidia || 
		devType == acc_device_radeon || devType == acc_device_current) {
    	CUresult cuResult = CUDA_SUCCESS;
        memPool_t *memPool = memPoolMap[tconf->threadID];
		sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
        std::multimap<size_t, void *>::iterator it = memPool->find(count);
        if (it != memPool->end()) {
#ifdef _OPENARC_PROFILE_
            if( HI_openarcrt_verbosity > 2 ) {
                fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_tempMalloc1D_async(%lu) reuses memories in the memPool\n", count);
            }
#endif           
            *tempPtr = it->second;
            memPool->erase(it);
			current_mempool_size -= count;
			(*tempMallocSize)[(const void *)*tempPtr] = count;
        } else { 
			if( current_mempool_size > tconf->max_mempool_size ) {
#ifdef _OPENARC_PROFILE_
                if( HI_openarcrt_verbosity > 2 ) {
                    fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_tempMalloc1D_async(%lu) releases memories in the memPool\n", count);
                }
#endif               
                for (it = memPool->begin(); it != memPool->end(); ++it) {
                    *tempPtr = it->second;
                    cuResult = cuMemFree((CUdeviceptr)(*tempPtr));
					current_mempool_size -= it->first;
#ifdef _OPENARC_PROFILE_ 
                    tconf->IDFreeCnt++;
        			tconf->CIDMemorySize -= it->first;
#endif                   
                    if(cuResult != CUDA_SUCCESS) {
                        fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D_async()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
                    }
                }        
                memPool->clear();
			}
            cuResult = cuMemAlloc((CUdeviceptr*)tempPtr, count);
            if (cuResult != CUDA_SUCCESS) {
#ifdef _OPENARC_PROFILE_
                if( HI_openarcrt_verbosity > 2 ) {
                    fprintf(stderr, "[OPENARCRT-INFO]\t\tCudaDriver::HI_tempMalloc1D_async(%lu) releases memories in the memPool\n", count);
                }
#endif               
                for (it = memPool->begin(); it != memPool->end(); ++it) {
                    *tempPtr = it->second;
                    cuResult = cuMemFree((CUdeviceptr)(*tempPtr));
					current_mempool_size -= it->first;
#ifdef _OPENARC_PROFILE_ 
                    tconf->IDFreeCnt++;
        			tconf->CIDMemorySize -= it->first;
#endif                   
                    if(cuResult != CUDA_SUCCESS) {
                        fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D_async()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
                    }
                }        
                memPool->clear();
                cuResult = cuMemAlloc((CUdeviceptr*)tempPtr, count);
            }        
    		if(cuResult == CUDA_SUCCESS) {
				//New temporary device memory is allocated.
				(*tempMallocSize)[(const void *)*tempPtr] = count;
#ifdef _OPENARC_PROFILE_
		        tconf->IDMallocCnt++;
        		tconf->IDMallocSize += count;
        		tconf->CIDMemorySize += count;
				if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
					tconf->MIDMemorySize = tconf->CIDMemorySize;
				}
#endif
			} else {
        		fprintf(stderr, "[ERROR in CudaDriver::HI_tempMalloc1D_async()] failed to malloc on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
    		}
        }        
    } else {
		if( tempMallocSet.count(*tempPtr) > 0 ) {
			tempMallocSet.erase(*tempPtr);	
            free(*tempPtr);
#ifdef _OPENARC_PROFILE_
            tconf->IHFreeCnt++;
#endif
        }
        *tempPtr = malloc(count);
		tempMallocSet.insert(*tempPtr);	
#ifdef _OPENARC_PROFILE_
        tconf->IHMallocCnt++;
        tconf->IHMallocSize += count;
        tconf->CIHMemorySize += count;
		if( tconf->MIHMemorySize < tconf->CIHMemorySize ) {
			tconf->MIHMemorySize = tconf->CIHMemorySize;
		}
#endif
    }
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_tempMalloc);
#endif
#ifdef _OPENARC_PROFILE_
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_tempMalloc1D_async()\n");
	}
#endif
}

//Used for de-allocating temporary data.
void CudaDriver::HI_tempFree( void** tempPtr, acc_device_t devType, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_tempFree(tempPtr = %lx, devType = %d, thread ID = %d)\n",  (long unsigned int)(*tempPtr), devType, threadID);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_tempMalloc);
#else
#ifdef _OPENMP
    #pragma omp critical (tempMalloc_critical)
#endif
#endif
	{
    if( devType == acc_device_gpu || devType == acc_device_nvidia 
    || devType == acc_device_radeon || devType == acc_device_current ) {
        if( *tempPtr != 0 ) {
            //We do not free the device memory; instead put it in the memory pool 
            memPool_t *memPool = memPoolMap[tconf->threadID];
			sizemap_t *tempMallocSize = tempMallocSizeMap[tconf->threadID];
			if( tempMallocSize->count((const void *)*tempPtr) == 0 ) {
				if( tempMallocSet.count(*tempPtr) > 0 ) {
					tempMallocSet.erase(*tempPtr);	
            		free(*tempPtr);
            		if( tconf->prepin_host_memory == 1 ) {
    					// Unpin host memory if already pinned.
    					HI_unpin_host_memory(*tempPtr, tconf->threadID);
					}
				} else {
					fprintf(stderr, "[OPENARCRT-WARNING in CudaDriver::HI_tempFree(devType = %d, thread ID = %d)] no tempMallocSize mapping found for tempPtr (%lx)\n", devType, threadID, (long unsigned int)(*tempPtr));
				}
			} else {
				size_t size = tempMallocSize->at((const void *)*tempPtr);
            	memPool->insert(std::pair<size_t, void *>(size, *tempPtr));
				current_mempool_size += size;
				tempMallocSize->erase((const void *)*tempPtr);
			}
        }
    } else {
        if( *tempPtr != 0 ) {
			tempMallocSet.erase(*tempPtr);	
            free(*tempPtr);
            if( tconf->prepin_host_memory == 1 ) {
    			// Unpin host memory if already pinned.
    			HI_unpin_host_memory(*tempPtr, tconf->threadID);
			}
#ifdef _OPENARC_PROFILE_
            tconf->IHFreeCnt++;
#endif
        }
    }
	}
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_tempMalloc);
#endif
    *tempPtr = 0;
#ifdef _OPENARC_PROFILE_
    tconf->totalFreeTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_tempFree(tempPtr = %lx, devType = %d, thread ID = %d)\n",  (long unsigned int)(*tempPtr), devType, threadID);
	}
#endif
}

#endif


//////////////////////
// Kernel Execution //
//////////////////////


//In the driver API, copying into a constant memory (symbol) does not require a different API call
HI_error_t  CudaDriver::HI_memcpy(void *dst, const void *src, size_t count, HI_MemcpyKind_t kind, int trType, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy(%lu)\n", count);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

    CUresult cuResult = CUDA_SUCCESS;
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
	if( dst != src ) {
    	switch( kind ) {
    	case HI_MemcpyHostToHost: {
			//[DEBUG] cuMemcpy works devices supporting unified addressing (compute capability >= 2.0)
        	cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        	break;
    	}
    	case HI_MemcpyHostToDevice: {
        	cuResult = cuMemcpyHtoD((CUdeviceptr) dst, src, count);
        	break;
    	}
    	case HI_MemcpyDeviceToHost: {
        	cuResult = cuMemcpyDtoH(dst, (CUdeviceptr)src, count);
        	break;
    	}
    	case HI_MemcpyDeviceToDevice: {
        	cuResult = cuMemcpyDtoD((CUdeviceptr) dst, (CUdeviceptr)src, count);
        	break;
    	}
    	}
	}
#ifdef _OPENARC_PROFILE_
	if( dst != src ) {
    	if( kind == HI_MemcpyHostToDevice ) {
        	tconf->H2DMemTrCnt++;
        	tconf->H2DMemTrSize += count;
    	} else if( kind == HI_MemcpyDeviceToHost ) {
        	tconf->D2HMemTrCnt++;
        	tconf->D2HMemTrSize += count;
    	} else if( kind == HI_MemcpyDeviceToDevice ) {
        	tconf->D2DMemTrCnt++;
        	tconf->D2DMemTrSize += count;
    	} else {
        	tconf->H2HMemTrCnt++;
        	tconf->H2HMemTrSize += count;
    	}
	}
    tconf->totalMemTrTime += HI_get_localtime() - ltime;
#endif
    if( cuResult == CUDA_SUCCESS ) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy(%lu)\n", count);
	}
#endif
        return HI_success;
    } else {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy()] Memcpy failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num());
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy()] Memcpy failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), 0);
		exit(1);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy(%lu)\n", count);
	}
#endif
        return HI_error;
    }
}

HI_error_t  CudaDriver::HI_memcpy_unified(void *dst, const void *src, size_t count, HI_MemcpyKind_t kind, int trType, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy_unified(%lu)\n", count);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

    CUresult cuResult = CUDA_SUCCESS;
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
	if( dst != src ) {
    	switch( kind ) {
    	case HI_MemcpyHostToHost: {
			//[DEBUG] cuMemcpy works devices supporting unified addressing (compute capability >= 2.0)
        	cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        	break;
    	}
    	case HI_MemcpyHostToDevice: {
        	//cuResult = cuMemcpyHtoD((CUdeviceptr) dst, src, count);
        	cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        	break;
    	}
    	case HI_MemcpyDeviceToHost: {
        	//cuResult = cuMemcpyDtoH(dst, (CUdeviceptr)src, count);
        	cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        	break;
    	}
    	case HI_MemcpyDeviceToDevice: {
        	//cuResult = cuMemcpyDtoD((CUdeviceptr) dst, (CUdeviceptr)src, count);
        	cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        	break;
    	}
    	}
	}
#ifdef _OPENARC_PROFILE_
	if( dst != src ) {
    	if( kind == HI_MemcpyHostToDevice ) {
        	tconf->H2DMemTrCnt++;
        	tconf->H2DMemTrSize += count;
    	} else if( kind == HI_MemcpyDeviceToHost ) {
        	tconf->D2HMemTrCnt++;
        	tconf->D2HMemTrSize += count;
    	} else if( kind == HI_MemcpyDeviceToDevice ) {
        	tconf->D2DMemTrCnt++;
        	tconf->D2DMemTrSize += count;
    	} else {
        	tconf->H2HMemTrCnt++;
        	tconf->H2HMemTrSize += count;
    	}
	}
    tconf->totalMemTrTime += HI_get_localtime() - ltime;
#endif
    if( cuResult == CUDA_SUCCESS ) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_unified(%lu)\n", count);
	}
#endif
        return HI_success;
    } else {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_unified()] Memcpy failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num());
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_unified()] Memcpy failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), 0);
		exit(1);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_unified(%lu)\n", count);
	}
#endif
        return HI_error;
    }
}

HI_error_t CudaDriver::HI_memcpy_const(void *hostPtr, std::string constName, HI_MemcpyKind_t kind, size_t count, int threadID) {
	void *devPtr;
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy_const(%lu)\n", count);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUresult cuResult;
    HI_error_t result = HI_success;
    CUdeviceptr dptr;
    size_t size;
	int asyncID = DEFAULT_QUEUE+tconf->asyncID_offset;
    cuResult = cuModuleGetGlobal( &dptr, &size, cuModule, constName.c_str());

//#ifdef _OPENARC_PROFILE_
//	double ltime = HI_get_localtime();
//#endif

    if( cuResult != CUDA_SUCCESS ) {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_const()] Acquiring constant memory handle failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num());
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_const()] Acquiring constant memory handle failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), 0);
		exit(1);
#endif
        result = HI_error;
    }

    result = HI_memcpy((void*)dptr, hostPtr, count, kind, 0);
    if(HI_get_device_address(hostPtr, &devPtr, NULL, NULL, asyncID, tconf->threadID) != HI_success ) {
		//Add to present table.
		devPtr = 0; //set to a fake address.
		HI_set_device_address(hostPtr, devPtr, (size_t) count, asyncID, tconf->threadID);
	}

//#ifdef _OPENARC_PROFILE_
//    tconf->totalMemTrTime += HI_get_localtime() - ltime;
//#endif

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_const(%lu)\n", count);
	}
#endif
    return result;
}

//[DEBUG] CUDA driver does not offer asynchronous version of cuModuleGetGlobal(), 
//and thus HI_memcpy_const_async() is the same as HI_memcpy_const().
HI_error_t CudaDriver::HI_memcpy_const_async(void *hostPtr, std::string constName, HI_MemcpyKind_t kind, size_t count, int async, int num_waits, int *waits, int threadID) {
	void *devPtr;
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy_const_async(%d, %lu)\n", async, count);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    HI_wait_for_events(async, num_waits, waits, tconf->threadID);

    CUresult cuResult;
    HI_error_t result = HI_success;
    CUdeviceptr dptr;
    size_t size;
    cuResult = cuModuleGetGlobal( &dptr, &size, cuModule, constName.c_str());

//#ifdef _OPENARC_PROFILE_
//	double ltime = HI_get_localtime();
//#endif

    if( cuResult != CUDA_SUCCESS ) {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_const_async()] Acquiring constant memory handle failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num());
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_const_async()] Acquiring constant memory handle failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), 0);
		exit(1);
#endif
        result = HI_error;
    }

    result = HI_memcpy((void*)dptr, hostPtr, count, kind, 0);
    if(HI_get_device_address(hostPtr, &devPtr, NULL, NULL, async, tconf->threadID) != HI_success ) {
		//Add to present table.
		devPtr = 0; //set to a fake address.
		HI_set_device_address(hostPtr, devPtr, (size_t) count, async, tconf->threadID);
	}

//#ifdef _OPENARC_PROFILE_
//    tconf->totalMemTrTime += HI_get_localtime() - ltime;
//#endif

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_const_async(%d, %lu)\n", async, count);
	}
#endif
    return result;
}

HI_error_t CudaDriver::HI_present_or_memcpy_const(void *hostPtr, std::string constName, HI_MemcpyKind_t kind, size_t count, int threadID) {
	void *devPtr;
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy_const(%lu)\n", count);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUresult cuResult;
    HI_error_t result = HI_success;
    CUdeviceptr dptr;
    size_t size;
	int asyncID = DEFAULT_QUEUE+tconf->asyncID_offset;
    cuResult = cuModuleGetGlobal( &dptr, &size, cuModule, constName.c_str());

//#ifdef _OPENARC_PROFILE_
//	double ltime = HI_get_localtime();
//#endif

    if( cuResult != CUDA_SUCCESS ) {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_const()] Acquiring constant memory handle failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num());
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_const()] Acquiring constant memory handle failed with error %d (%s) in tid %d\n", cuResult, cuda_error_code(cuResult), 0);
		exit(1);
#endif
        result = HI_error;
    }

    if(HI_get_device_address(hostPtr, &devPtr, NULL, NULL, asyncID, tconf->threadID) != HI_success ) {
    	result = HI_memcpy((void*)dptr, hostPtr, count, kind, 0);
		//Add to present table.
		devPtr = 0; //set to a fake address.
		HI_set_device_address(hostPtr, devPtr, (size_t) count, asyncID, tconf->threadID);
	}

//#ifdef _OPENARC_PROFILE_
//    tconf->totalMemTrTime += HI_get_localtime() - ltime;
//#endif

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_const(%lu)\n", count);
	}
#endif
    return result;
}


HI_error_t CudaDriver::HI_memcpy_async(void *dst, const void *src, size_t count,
        HI_MemcpyKind_t kind, int trType, int async, int num_waits, int *waits, int threadID) {
    HostConf_t * tconf = getHostConf(threadID);

/*
	if( tconf->prepin_host_memory == 0 ) {
		fprintf(stderr, "[ERROR CudaDriver::HI_memcpy_async()] To use asynchronous transfers in CUDA GPUs, environment variable OPENARCRT_PREPINHOSTMEM should be set to 1; exit!\n");
		exit(1);
	}
*/

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy_async(%d, %lu)\n", async, count);
	}
#endif
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    HI_wait_for_events(async, num_waits, waits, tconf->threadID);

    CUresult cuResult = CUDA_SUCCESS;
    CUstream stream = getQueue(async, tconf->threadID);
    CUevent event = getEvent(async, tconf->threadID);
	const void * baseHostPtr = 0;

	if( dst != src ) {
    	switch( kind ) {
    	case HI_MemcpyHostToHost: {
			//[DEBUG] cuMemcpy works devices supporting unified addressing (compute capability >= 2.0)
        	cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        	break;
    	}
    	case HI_MemcpyHostToDevice: {
			baseHostPtr = HI_get_base_address_of_host_memory(src, async, tconf->threadID);
			if( baseHostPtr == 0 ) {
				baseHostPtr = src;
			}
			pin_host_memory_if_unpinned(baseHostPtr, count);
        	cuResult = cuMemcpyHtoDAsync((CUdeviceptr) dst, src, count, stream);
        	break;
    	}
    	case HI_MemcpyDeviceToHost: {
			baseHostPtr = HI_get_base_address_of_host_memory((const void *)dst, async, tconf->threadID);
			if( baseHostPtr == 0 ) {
				baseHostPtr = (const void *)dst;
			}
			pin_host_memory_if_unpinned(baseHostPtr, count);
        	cuResult = cuMemcpyDtoHAsync(dst, (CUdeviceptr)src, count, stream);
        	break;
    	}
    	case HI_MemcpyDeviceToDevice: {
        	cuResult = cuMemcpyDtoDAsync((CUdeviceptr) dst, (CUdeviceptr)src, count, stream);
        	break;
    	}
    	}
	}

    cuEventRecord(event, stream);
#ifdef _OPENARC_PROFILE_
	if( dst != src ) {
    	if( kind == HI_MemcpyHostToDevice ) {
        	tconf->H2DMemTrCnt++;
        	tconf->H2DMemTrSize += count;
    	} else if( kind == HI_MemcpyDeviceToHost ) {
        	tconf->D2HMemTrCnt++;
        	tconf->D2HMemTrSize += count;
    	} else if( kind == HI_MemcpyDeviceToDevice ) {
        	tconf->D2DMemTrCnt++;
        	tconf->D2DMemTrSize += count;
    	} else {
        	tconf->H2HMemTrCnt++;
        	tconf->H2HMemTrSize += count;
    	}
	}
    tconf->totalMemTrTime += HI_get_localtime() - ltime;
#endif
    if( cuResult == CUDA_SUCCESS ) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_async(%d, %lu)\n", async, count);
	}
#endif
        return HI_success;
    } else {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_async()] Memcpy failed with error %d (%s) in tid %d with asyncId %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num(), async);
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_async()] Memcpy failed with error %d (%s) in tid %d with asyncId %d\n", cuResult, cuda_error_code(cuResult), 0, async);
		exit(1);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_async(%d, %lu)\n", async, count);
	}
#endif
        return HI_error;
    }
}

//Used for kernel verification.
HI_error_t CudaDriver::HI_memcpy_asyncS(void *dst, const void *src, size_t count,
        HI_MemcpyKind_t kind, int trType, int async, int num_waits, int *waits, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy_asyncS(%d)\n", async);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    HI_wait_for_events(async, num_waits, waits, tconf->threadID);

    CUresult cuResult;
    CUstream stream = getQueue(async, tconf->threadID);
    CUevent event = getEvent(async, tconf->threadID);

    switch( kind ) {
    case HI_MemcpyHostToHost: {
		//[DEBUG] cuMemcpy works devices supporting unified addressing (compute capability >= 2.0)
        cuResult = cuMemcpy((CUdeviceptr) dst, (CUdeviceptr) src, count);
        break;
    }
    case HI_MemcpyHostToDevice: {
		void *tSrc = 0;
		HI_tempMalloc1D(&tSrc, count, acc_device_host, HI_MEM_READ_WRITE);
		if( tconf->prepin_host_memory == 1 ) {
        	//Pin host memory
        	if( HI_pin_host_memory(tSrc, (size_t) count, tconf->threadID) == HI_error ) {
#ifdef _OPENMP
                fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", omp_get_thread_num());
				exit(1);
#else
				fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", tconf->threadID);
				exit(1);
#endif
			}
		}
        cuResult = cuMemcpy((CUdeviceptr) tSrc, (CUdeviceptr) src, count);
        cuResult = cuMemcpyHtoDAsync((CUdeviceptr) dst, tSrc, count, stream);
        break;
    }
    case HI_MemcpyDeviceToHost: {
		void *tDst = 0;
		HI_tempMalloc1D(&tDst, count, acc_device_host, HI_MEM_READ_WRITE);
		if( tconf->prepin_host_memory == 1 ) {
        	//Pin host memory
        	if( HI_pin_host_memory(tDst, (size_t) count, tconf->threadID) == HI_error ) {
#ifdef _OPENMP
                fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", omp_get_thread_num());
				exit(1);
#else
				fprintf(stderr, "[ERROR in CudaDriver::HI_pin_host_memory()] Cannot pin host memory by tid: %d\n", tconf->threadID);
				exit(1);
#endif
			}
		}
		HI_set_temphost_address(dst, tDst, async);
        cuResult = cuMemcpyDtoHAsync(tDst, (CUdeviceptr)src, count, stream);
        break;
    }
    case HI_MemcpyDeviceToDevice: {
        cuResult = cuMemcpyDtoDAsync((CUdeviceptr) dst, (CUdeviceptr)src, count, stream);
        break;
    }
    }

    cuEventRecord(event, stream);
#ifdef _OPENARC_PROFILE_
    if( kind == HI_MemcpyHostToDevice ) {
        tconf->H2DMemTrCnt++;
        tconf->H2DMemTrSize += count;
    } else if( kind == HI_MemcpyDeviceToHost ) {
        tconf->D2HMemTrCnt++;
        tconf->D2HMemTrSize += count;
    } else if( kind == HI_MemcpyDeviceToDevice ) {
        tconf->D2DMemTrCnt++;
        tconf->D2DMemTrSize += count;
    } else {
        tconf->H2HMemTrCnt++;
        tconf->H2HMemTrSize += count;
    }
    tconf->totalMemTrTime += HI_get_localtime() - ltime;
#endif
    if( cuResult == CUDA_SUCCESS ) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_asyncS(%d)\n", async);
	}
#endif
        return HI_success;
    } else {
#ifdef _OPENMP
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_asyncS()] Memcpy failed with error %d (%s) in tid %d with asyncId %d\n", cuResult, cuda_error_code(cuResult), omp_get_thread_num(), async);
		exit(1);
#else
        fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy_asyncS()] Memcpy failed with error %d (%s) in tid %d with asyncId %d\n", cuResult, cuda_error_code(cuResult), 0, async);
		exit(1);
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy_asyncS(%d)\n", async);
	}
#endif
        return HI_error;
    }
}


HI_error_t CudaDriver::HI_memcpy2D(void *dst, size_t dpitch, const void *src, size_t spitch,
        size_t widthInBytes, size_t height, HI_MemcpyKind_t kind, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy2D()\n");
	}
	
#endif
    HostConf_t * tconf = getHostConf(threadID);

#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    CUresult cuResult=CUDA_ERROR_DEINITIALIZED;
    CUDA_MEMCPY2D pcopy;
    switch( kind ) {
    case HI_MemcpyHostToHost: {
        pcopy.srcMemoryType =  CU_MEMORYTYPE_HOST;
        pcopy.dstMemoryType =  CU_MEMORYTYPE_HOST;
        pcopy.srcHost = src;
        pcopy.dstHost = dst;
        break;
    }
    case HI_MemcpyHostToDevice: {
        pcopy.srcMemoryType =  CU_MEMORYTYPE_HOST;
        pcopy.dstMemoryType =  CU_MEMORYTYPE_DEVICE;
        pcopy.srcHost = src;
        pcopy.dstDevice = (CUdeviceptr) dst;
        break;
    }
    case HI_MemcpyDeviceToHost: {
        pcopy.srcMemoryType =  CU_MEMORYTYPE_DEVICE;
        pcopy.dstMemoryType =  CU_MEMORYTYPE_HOST;
        pcopy.srcDevice = (CUdeviceptr) src;
        pcopy.dstHost = dst;
        break;
    }
    case HI_MemcpyDeviceToDevice: {
        pcopy.srcMemoryType =  CU_MEMORYTYPE_DEVICE;
        pcopy.dstMemoryType =  CU_MEMORYTYPE_DEVICE;
        pcopy.srcDevice = (CUdeviceptr) src;
        pcopy.dstDevice = (CUdeviceptr) dst;
        break;
    }
    }

    pcopy.srcXInBytes = 0;
    pcopy.srcY = 0;
    pcopy.dstXInBytes = 0;
    pcopy.dstY = 0;
    pcopy.srcPitch = spitch;
    pcopy.dstPitch = dpitch;
    pcopy.WidthInBytes = widthInBytes;
    pcopy.Height = height;

    cuResult = cuMemcpy2D(&pcopy);
    //fprintf(stderr, "[in HI_memcpy2D()] Memcpy done\n");
#ifdef _OPENARC_PROFILE_
    if( kind == HI_MemcpyHostToDevice ) {
        tconf->H2DMemTrCnt++;
        tconf->H2DMemTrSize += widthInBytes*height;
    } else if( kind == HI_MemcpyDeviceToHost ) {
        tconf->D2HMemTrCnt++;
        tconf->D2HMemTrSize += widthInBytes*height;
    } else if( kind == HI_MemcpyDeviceToDevice ) {
        tconf->D2DMemTrCnt++;
        tconf->D2DMemTrSize += widthInBytes*height;
    } else {
        tconf->H2HMemTrCnt++;
        tconf->H2HMemTrSize += widthInBytes*height;
    }
    tconf->totalMemTrTime += HI_get_localtime() - ltime;
#endif
    if( cuResult == CUDA_SUCCESS ) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy2D()\n");
	}
#endif
        return HI_success;
    } else {
				fprintf(stderr, "[ERROR in CudaDriver::HI_memcpy2D()] Memcpy failed with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
				exit(1);
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy2D()\n");
	}
#endif
        return HI_error;
    }
}

HI_error_t CudaDriver::HI_memcpy2D_async(void *dst, size_t dpitch, const void *src,
        size_t spitch, size_t widthInBytes, size_t height, HI_MemcpyKind_t kind, int async, int num_waits, int *waits, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_memcpy2D_async(%d)\n", async);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    /*
    #ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
    #endif
    CUresult cuResult;
    //acc_device_t devType = acc_get_device_type(tconf->threadID);
    acc_device_t devType = tconf->acc_device_type_var;
    int devNum = acc_get_device_num(devType);
    cudaStream_t stream;
    asyncmap_t * asyncmap = tconf->asyncmaptable;

    if( asyncmap->count(async) > 0 ) {
    	stream = asyncmap->at(async);
    } else {
    	cudaStreamCreate(&stream);
    	(*asyncmap)[async] = stream;
    }

    void * dstT;
    const void * srcT;
    void * pinnedHostPtr;
    size_t count;
    if( kind == HI_MemcpyHostToHost ) {
    	CUDA_MEMCPY2D pcopy;
    	pcopy.srcMemoryType =  CU_MEMORYTYPE_HOST;
    	pcopy.dstMemoryType =  CU_MEMORYTYPE_HOST;
    	pcopy.srcHost = src;
    	pcopy.dstHost = dst;
    	pcopy.srcXInBytes = 0;
    	pcopy.srcY = 0;
    	pcopy.dstXInBytes = 0;
    	pcopy.dstY = 0;
    	pcopy.srcPitch = spitch;
    	pcopy.dstPitch = dpitch;
    	pcopy.WidthInBytes = widthInBytes;
    	pcopy.Height = height;
    	cuResult = cuMemcpy2D(&pcopy);
    } else {
    	if( kind == HI_MemcpyHostToDevice ) {
    		//cuResult = cudaMallocHost(&pinnedHostPtr, count);
    		count = spitch*height;
    		pinnedHostPtr = tconf->get_asyncphostaddress(devNum, async, src, count);
    		if( tconf->has_pendingphost2hostcopy(devNum, async, src) == 0 ) {
    			//cuResult = cudaMemcpy(pinnedHostPtr, src, count, cudaMemcpyHostToHost);
    			cuResult = cuMemcpyHtoD((CUdeviceptr)pinnedHostPtr, src, count);
    		}
    		srcT = pinnedHostPtr;
    		dstT = dst;
    	} else if( kind == HI_MemcpyDeviceToHost ) {
    		//cuResult = cudaMallocHost(&pinnedHostPtr, count);
    		count = dpitch*height;
    		pinnedHostPtr = tconf->get_asyncphostaddress(devNum, async, dst, count);
    		dstT = pinnedHostPtr;
    		srcT = src;
    	} else {
    		dstT = dst;
    		srcT = src;
    	}

    	CUDA_MEMCPY2D pcopy;
    	switch( kind ) {
    		case HI_MemcpyHostToHost: {pcopy.srcMemoryType =  CU_MEMORYTYPE_HOST;
    									  pcopy.dstMemoryType =  CU_MEMORYTYPE_HOST;
    									  pcopy.srcHost = src;
    									  pcopy.dstHost = dst;}
    		case HI_MemcpyHostToDevice: {pcopy.srcMemoryType =  CU_MEMORYTYPE_HOST;
    										pcopy.dstMemoryType =  CU_MEMORYTYPE_DEVICE;
    										pcopy.srcHost = src;
    										pcopy.dstDevice = (CUdeviceptr) dst;}
    		case HI_MemcpyDeviceToHost: {pcopy.srcMemoryType =  CU_MEMORYTYPE_DEVICE;
    										pcopy.dstMemoryType =  CU_MEMORYTYPE_HOST;
    										pcopy.srcDevice = (CUdeviceptr) src;
    										pcopy.dstHost = dst;}
    		case HI_MemcpyDeviceToDevice: {pcopy.srcMemoryType =  CU_MEMORYTYPE_DEVICE;
    										  pcopy.dstMemoryType =  CU_MEMORYTYPE_DEVICE;
    										  pcopy.srcDevice = (CUdeviceptr) src;
    										  pcopy.dstDevice = (CUdeviceptr) dst;}
    	}

    	pcopy.srcXInBytes = 0;
    	pcopy.srcY = 0;
    	pcopy.dstXInBytes = 0;
    	pcopy.dstY = 0;
    	pcopy.srcPitch = spitch;
    	pcopy.dstPitch = dpitch;
    	pcopy.WidthInBytes = widthInBytes;
    	pcopy.Height = height;

    	cuResult = cuMemcpy2DAsync(&pcopy, stream);

    	//cuResult = cudaMemcpy2DAsync(dstT, dpitch, srcT, spitch, widthInBytes, height, toCudaMemcpyKind(kind), stream);
    	if( kind == HI_MemcpyHostToDevice ) {
    		//cuResult = cudaFreeHost(pinnedHostPtr);
    		tconf->set_asynchostphostmap(devNum, async, src, pinnedHostPtr);
    	} else if( kind == HI_MemcpyDeviceToHost ) {
    		//cuResult = cudaMemcpy(dst, pinnedHostPtr, count, cudaMemcpyHostToHost);
    		tconf->set_asynchostsizemap(devNum, async, dst, count);
    		//cuResult = cudaFreeHost(pinnedHostPtr);
    		tconf->set_asynchostphostmap(devNum, async, dst, pinnedHostPtr);
    	}
    }
    #ifdef _OPENARC_PROFILE_
    if( kind == HI_MemcpyHostToDevice ) {
    	tconf->H2DMemTrCnt++;
    	tconf->H2DMemTrSize += widthInBytes*height;
    } else if( kind == HI_MemcpyDeviceToHost ) {
    	tconf->D2HMemTrCnt++;
    	tconf->D2HMemTrSize += widthInBytes*height;
    } else if( kind == HI_MemcpyDeviceToDevice ) {
    	tconf->D2DMemTrCnt++;
    	tconf->D2DMemTrSize += widthInBytes*height;
    } else {
    	tconf->H2HMemTrCnt++;
    	tconf->H2HMemTrSize += widthInBytes*height;
    }
    tconf->totalMemTrTime += HI_get_localtime() - ltime;
    #endif
    if( cuResult == CUDA_SUCCESS ) {
    	return HI_success;
    } else {
    	return HI_error;
    }

    */
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_memcpy2D_async(%d)\n", async);
	}
#endif
    return HI_success;
}

HI_error_t CudaDriver::HI_register_kernel_numargs(std::string kernel_name, int num_args, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_register_kernel_numargs()\n");
	}
#endif
    HostConf_t *tconf = getHostConf(threadID);
	//fprintf(stderr, "find kernelargs map for the current device\n");
    //(tconf->kernelArgsMap.at(this).at(kernel_name))->insert(std::pair<int, argument_t>(arg_index, arg));
/*
	if( tconf->kernelArgsMap.count(this) == 0 ) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_register_kernel_numargs(%s, %d)] kernelArgsMap does not exist for the current device.\n",kernel_name.c_str(), num_args);
			exit(1);
	} else if( tconf->kernelArgsMap.at(this).count(kernel_name) == 0 ) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_register_kernel_numargs(%s, %d)] kernelArgsMap does not have an entry for the current kernel.\n",kernel_name.c_str(), num_args);
			exit(1);
	}
*/
	kernelParams_t *kernelParams = tconf->kernelArgsMap.at(this).at(kernel_name);
	if( kernelParams->num_args == 0 ) {
		if( num_args > 0 ) {
			kernelParams->num_args = num_args;
    		kernelParams->kernelParams = (void**)malloc(sizeof(void*) * num_args);
		} else {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_register_kernel_numargs(%s, %d)] num_args should be greater than zero.\n",kernel_name.c_str(), num_args);
			exit(1);
		}
	}
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_register_kernel_numargs()\n");
	}
#endif
    return HI_success;
}


HI_error_t CudaDriver::HI_register_kernel_arg(std::string kernel_name, int arg_index, size_t arg_size, void *arg_value, int arg_type, int threadID)
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_register_kernel_arg()\n");
	}
#endif
    HostConf_t *tconf = getHostConf(threadID);
	//fprintf(stderr, "find kernelargs map for the current device\n");

	kernelParams_t * kernelParams = tconf->kernelArgsMap.at(this).at(kernel_name);
	if( kernelParams->num_args > arg_index ) {
		*(kernelParams->kernelParams + arg_index) = arg_value;
	} else {
		fprintf(stderr, "[ERROR in CudaDriver::HI_register_kernel_arg()] Kernel %s is registered to have %d arguments, but the current argument index %d is out of the bound.\n",kernel_name.c_str(), kernelParams->num_args, arg_index);
		exit(1);
	}

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_register_kernel_arg()\n");
	}
#endif
//    debug_arg = argumentMap->at(arg_index);
//    printf("debug arg:\t %d %e %d %X %X\n", arg_index, *((double*)debug_arg.arg_val), *((int*)debug_arg.arg_val), debug_arg.arg_val, argumentMap);

    return HI_success;
}



HI_error_t CudaDriver::HI_kernel_call(std::string kernel_name, size_t gridSize[3], size_t blockSize[3], int async, int num_waits, int *waits, int threadID)
{
	const char* c_kernel_name = kernel_name.c_str();
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_kernel_call(%s, %d)\n",c_kernel_name, async);
	}
#endif
	if( (gridSize[0] > maxGridX) || (gridSize[1] > maxGridY) || (gridSize[2] > maxGridZ) ) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_kernel_call()] Kernel [%s] Launch FAIL due to too large Grid configuration (%lu, %lu, %lu); exit!\n", c_kernel_name, gridSize[2], gridSize[1], gridSize[0]);
		exit(1);
	}
	if( (blockSize[0] > maxBlockX) || (blockSize[1] > maxBlockY) || (blockSize[2] > maxBlockZ) || (blockSize[0]*blockSize[1]*blockSize[2] > maxNumThreadsPerBlock) ) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_kernel_call()] Kernel [%s] Launch FAIL due to too large threadBlock configuration (%lu, %lu, %lu); exit!\n",c_kernel_name, blockSize[2], blockSize[1], blockSize[0]);
		exit(1);
	}
    HostConf_t *tconf = getHostConf(threadID);
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif

    HI_wait_for_events(async, num_waits, waits, tconf->threadID);

    CUresult err;
    //fprintf(stderr, "[HI_kernel_call()] GRIDSIZE %d %d %d\n", gridSize[2], gridSize[1], gridSize[0]);
    CUstream stream = getQueue(async, tconf->threadID);
    CUevent event = getEvent(async, tconf->threadID);
    if(async != DEFAULT_QUEUE+tconf->asyncID_offset) {
        err = cuLaunchKernel(tconf->kernelsMap.at(this).at(kernel_name), gridSize[0], gridSize[1], gridSize[2], blockSize[0], blockSize[1], blockSize[2], 0, stream, (tconf->kernelArgsMap.at(this).at(kernel_name))->kernelParams, NULL);

        cuEventRecord(event, stream);

    } else {
        err = cuLaunchKernel(tconf->kernelsMap.at(this).at(kernel_name), gridSize[0], gridSize[1], gridSize[2], blockSize[0], blockSize[1], blockSize[2], 0, 0, (tconf->kernelArgsMap.at(this).at(kernel_name))->kernelParams, NULL);
    }
    if (err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_kernel_call(%s)] Kernel Launch FAIL with error %d (%s)\n",c_kernel_name, err, cuda_error_code(err));
	fprintf(stderr, "\tgrid configuration: %lu %lu %lu\n", gridSize[2], gridSize[1], gridSize[0]);
	fprintf(stderr, "\tthread block configuration: %lu %lu %lu\n", blockSize[2], blockSize[1], blockSize[0]);
		exit(1);
        return HI_error;
    }

#ifdef _OPENARC_PROFILE_
	if(tconf->KernelCNTMap.count(kernel_name) == 0) {
		tconf->KernelCNTMap[kernel_name] = 0;
	}
    tconf->KernelCNTMap[kernel_name] += 1;
	if(tconf->KernelTimingMap.count(kernel_name) == 0) {
		tconf->KernelTimingMap[kernel_name] = 0.0;
	}
    err = cuStreamSynchronize(stream);
    err = cuStreamSynchronize(0);
    tconf->KernelTimingMap[kernel_name] += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_kernel_call(%s, %d)\n",c_kernel_name, async);
	}
#endif
    return HI_success;
}

//If unified memory is not used, the default queue will handle necessary
//synchronization, and thus no need for this explicit synchronization.
//However, explicit synchronization for the default queue  will be necessary 
//before any asynchronous operations on non-default queue, since they may 
//use results from the operations in  the default queue.
//To handle this case, explicit synchronization is added before HI_set_async().
// ==> In CUDA, NULL stream waits on all blocking streams, and all blocking
//     streams wait on the NULL stream. Therefore, we don't need any explicit
//     synchronization for any blocking stream. (a blocking stream
//     is created when cuStreamCreate() is called with a default flag.)
HI_error_t CudaDriver::HI_synchronize( int forcedSync, int threadID )
{
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_synchronize(%d)\n", forcedSync);
	}
#endif
	if( (forcedSync != 0) || (unifiedMemSupported == 1) ) {
		//cuCtxSynchronize() waits for all tasks in the current context, but we
		//need to wait for the tasks in the default queue (NULL stream).
    	//CUresult err2 = cuCtxSynchronize();
    	//if (err2 != CUDA_SUCCESS) {
        //	fprintf(stderr, "[ERROR in CudaDriver::HI_synchronize()] Current Context Synchronization FAIL with error %d (%s)\n", err2, cuda_error_code(err2));
		//	exit(1);
        //	return HI_error;
    	//}
    	HostConf_t * tconf = getHostConf(threadID);
    	CUstream stream = getQueue(DEFAULT_QUEUE+tconf->asyncID_offset, tconf->threadID);
    	CUresult err = cuStreamSynchronize(stream);
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_synchronize()] DEFAULT_QUEUE Context Synchronization FAIL with error %d (%s)\n", err, cuda_error_code(err));
			exit(1);
        	return HI_error;
    	}
		if( stream != 0 ) {
    		err = cuStreamSynchronize(0);
    		if (err != CUDA_SUCCESS) {
        		fprintf(stderr, "[ERROR in CudaDriver::HI_synchronize()] Defalut Context Synchronization FAIL with error %d (%s)\n", err, cuda_error_code(err));
				exit(1);
        		return HI_error;
    		}
		}
	}

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_synchronize(%d)\n", forcedSync);
	}
#endif
    return HI_success;
}



HI_error_t CudaDriver::HI_bind_tex(std::string texName,  HI_datatype_t type, const void *devPtr, size_t size, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_bind_tex()\n");
	}
#endif
	if( size > max1DTexRefWidth4LM ) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] Texture binding for %s failed due to too large size (%ld); exit!\n",texName.c_str(), size);
		exit(1);
	}

    CUresult err;
    CUtexref cuTexref;
    HI_error_t result = HI_success;
    err = cuModuleGetTexRef(&cuTexref, cuModule, texName.c_str());
    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] failed to find CUDA texture '%s' with error %d (%s)\n", texName.c_str(), err, cuda_error_code(err));
		exit(1);
    }
    err = cuTexRefSetAddress(0, cuTexref, (CUdeviceptr)devPtr, size);
    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] failed to set address for CUDA texture '%s' with error %d (%s)\n", texName.c_str(), err, cuda_error_code(err));
		exit(1);
    }
    err = cuTexRefSetAddressMode(cuTexref, 0, CU_TR_ADDRESS_MODE_WRAP);
    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] failed to set address mode for CUDA texture '%s' with error %d (%s)\n", texName.c_str(), err, cuda_error_code(err));
		exit(1);
    }
    err = cuTexRefSetFilterMode(cuTexref, CU_TR_FILTER_MODE_LINEAR);
    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] failed to set filter mode for CUDA texture '%s' with error %d (%s)\n", texName.c_str(), err, cuda_error_code(err));
		exit(1);
    }
    err = cuTexRefSetFlags(cuTexref, CU_TRSF_NORMALIZED_COORDINATES);
    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] failed to set flags for CUDA texture '%s' with error %d (%s)\n", texName.c_str(), err, cuda_error_code(err));
		exit(1);
    }

    if(type == HI_int) {
        err = cuTexRefSetFormat(cuTexref, CU_AD_FORMAT_SIGNED_INT32, 1);
    } else if (type == HI_float) {
        err = cuTexRefSetFormat(cuTexref, CU_AD_FORMAT_FLOAT, 1);
    } else {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] Unsupported format for CUDA texture '%s' (NVIDIA CUDA GPU)\n", texName.c_str());
		exit(1);
        result = HI_error;
    }

    if(err != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_bind_tex()] failed to set format for CUDA texture '%s' with error %d (%s)\n", texName.c_str(), err, cuda_error_code(err));
		exit(1);
        result = HI_error;
    }

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_bind_tex()\n");
	}
#endif
    return result;
}

void CudaDriver::HI_set_async(int asyncId, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_set_async(%d)\n", asyncId);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    int thread_id = tconf->threadID;
#ifdef _THREAD_SAFETY
        pthread_mutex_lock(&mutex_set_async);
#else
#ifdef _OPENMP
    #pragma omp critical (HI_set_async_critical)
#endif
#endif
    {
        asyncId += 2;
        std::map<int, CUstream >::iterator it= queueMap.find(asyncId);

        if(it == queueMap.end()) {
            CUstream str;
			// CU_STREAM_DEFAULT => create a blocking stream that synchronizes 
			// with NULL stream.
			// CU_STREAM_NON_BLOCKING => create a non-blocking stream that may 
			// run concurrently with the NULL stream (no implicit synchronization 
			// with the NULL stream).
#ifdef USE_BLOCKING_STREAMS
    		cuStreamCreate(&str, CU_STREAM_DEFAULT);
#else
            cuStreamCreate(&str, CU_STREAM_NON_BLOCKING);
#endif
            queueMap[asyncId] = str;
        }

        std::map<int, std::map<int, CUevent> >::iterator threadIt;
        threadIt = threadQueueEventMap.find(thread_id);

        //threadQueueEventMap is empty for this thread
        if(threadIt == threadQueueEventMap.end()) {
            std::map<int, CUevent> newMap;
            CUevent ev;
            cuEventCreate(&ev, CU_EVENT_DEFAULT);
            newMap[asyncId] = ev;
            threadQueueEventMap[thread_id] = newMap;
        } else {
            //threadQueueEventMap does not have an entry for this stream
            //std::map<int, CUevent> evMap = threadIt->second;
            if(threadIt->second.find(asyncId) == threadIt->second.end()) {
                CUevent ev;
                cuEventCreate(&ev, CU_EVENT_DEFAULT);
                threadIt->second[asyncId] = ev;
                //threadIt->second = evMap;
            }
        }
    }
#ifdef _THREAD_SAFETY
        pthread_mutex_unlock(&mutex_set_async);
#endif
#ifndef USE_BLOCKING_STREAMS
	if( unifiedMemSupported == 0 ) {
		//We need explicit synchronization here for the default queue 
		//since HI_synchronize() does not explicitly synchronize if 
		//unified memory is not used.
    	//CUresult err = cuCtxSynchronize();
    	//CUresult err = cuStreamSynchronize(getQueue(DEFAULT_QUEUE+tconf->asyncID_offset, tconf->threadID));
    	//err = cuStreamSynchronize(0);
    	CUstream stream = getQueue(DEFAULT_QUEUE+tconf->asyncID_offset, tconf->threadID);
    	CUresult err = cuStreamSynchronize(stream);
    	if (err != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_set_async()] DEFAULT_QUEUE Context Synchronization FAIL with error %d (%s)\n", err, cuda_error_code(err));
			exit(1);
    	}
		if( stream != 0 ) {
    		err = cuStreamSynchronize(0);
    		if (err != CUDA_SUCCESS) {
        		fprintf(stderr, "[ERROR in CudaDriver::HI_set_async()] Default Context Synchronization FAIL with error %d (%s)\n", err, cuda_error_code(err));
				exit(1);
    		}
		}
	}
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_set_async(%d)\n", asyncId-2);
	}
#endif
}

void CudaDriver::HI_set_context(int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_set_context()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
	CUcontext tContext;
	//Check whether the current host thread has a CUDA context.
    CUresult err = cuCtxGetCurrent(&tContext);
    if((err == CUDA_SUCCESS) && (tContext != NULL)) {
		//If existing, use the current CUDA context.
		cuCtxSetCurrent(tContext);
	} else if( cuContext != NULL) {
		cuCtxSetCurrent(cuContext);
	} else {
        fprintf(stderr, "[ERROR in CudaDriver::HI_set_context()] cannot find a valid CUDA context; exit!\n");
		exit(1);
	}

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_set_context()\n");
	}
#endif
}

void CudaDriver::HI_wait(int arg, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait(%d)\n", arg);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent(arg, tconf->threadID);

    CUresult cuResult = cuEventSynchronize(event);

    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_wait()] failed wait on CUDA queue %d with error %d (%s)\n", arg, cuResult, cuda_error_code(cuResult));
		exit(1);
    }

    HI_postponed_free(arg, tconf->threadID);
    HI_postponed_tempFree(arg, tconf->acc_device_type_var, tconf->threadID);
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait(%d)\n", arg);
	}
#endif
}

void CudaDriver::HI_wait_ifpresent(int arg, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait_ifpresent(%d)\n", arg);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent_ifpresent(arg, tconf->threadID);
	if( event != NULL ) {

    	CUresult cuResult = cuEventSynchronize(event);

    	if(cuResult != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_wait_ifpresent()] failed wait on CUDA queue %d with error %d (%s)\n", arg, cuResult, cuda_error_code(cuResult));
			exit(1);
    	}

    	HI_postponed_free(arg, tconf->threadID);
    	HI_postponed_tempFree(arg, tconf->acc_device_type_var, tconf->threadID);
	}
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait_ifpresent(%d)\n", arg);
	}
#endif
}

//[DEBUG] Below implementation is inefficient.
void CudaDriver::HI_wait_async(int arg, int async, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait_async(%d, %d)\n", arg, async);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent(arg, tconf->threadID);
    CUevent event2 = getEvent(async, tconf->threadID);

    CUresult cuResult = cuEventSynchronize(event);

    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_wait_async()] failed wait on CUDA queue %d with error %d (%s)\n", arg, cuResult, cuda_error_code(cuResult));
		exit(1);
    }

    HI_postponed_free(arg, tconf->threadID);
    HI_postponed_tempFree(arg, tconf->acc_device_type_var, tconf->threadID);

    cuResult = cuEventSynchronize(event2);

    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_wait_async()] failed wait on CUDA queue %d with error %d (%s)\n", async, cuResult, cuda_error_code(cuResult));
		exit(1);
    }

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait_async(%d, %d)\n", arg, async);
	}
#endif
}

void CudaDriver::HI_wait_async_ifpresent(int arg, int async, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait_async_ifpresent(%d, %d)\n", arg, async);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent_ifpresent(arg, tconf->threadID);
    CUevent event2 = getEvent_ifpresent(async, tconf->threadID);
	if( (event != NULL) && (event2 != NULL) ) {

    	CUresult cuResult = cuEventSynchronize(event);

    	if(cuResult != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_wait_async_ifpresent()] failed wait on CUDA queue %d with error %d (%s)\n", arg, cuResult, cuda_error_code(cuResult));
			exit(1);
    	}

    	HI_postponed_free(arg, tconf->threadID);
    	HI_postponed_tempFree(arg, tconf->acc_device_type_var, tconf->threadID);

    	cuResult = cuEventSynchronize(event2);

    	if(cuResult != CUDA_SUCCESS) {
        	fprintf(stderr, "[ERROR in CudaDriver::HI_wait_async_ifpresent()] failed wait on CUDA queue %d with error %d (%s)\n", async, cuResult, cuda_error_code(cuResult));
			exit(1);
    	}
	}

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait_async_ifpresent(%d, %d)\n", arg, async);
	}
#endif
}

void CudaDriver::HI_waitS1(int asyncId, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_waitS1(%d)\n", asyncId);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent(asyncId, tconf->threadID);

    CUresult cuResult = cuEventSynchronize(event);

    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_waitS1()] failed wait on CUDA queue %d with error %d (%s)\n", asyncId, cuResult, cuda_error_code(cuResult));
		exit(1);
    }

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_waitS1(%d)\n", asyncId);
	}
#endif
}

void CudaDriver::HI_waitS2(int asyncId, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_waitS2(%d)\n", asyncId);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
	HI_free_temphosts(asyncId);
    HI_postponed_free(asyncId, tconf->threadID);
    HI_postponed_tempFree(asyncId, tconf->acc_device_type_var, tconf->threadID);
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_waitS2(%d)\n", asyncId);
	}
#endif
}

void CudaDriver::HI_wait_all(int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait_all()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    eventmap_cuda_t *eventMap = &threadQueueEventMap.at(tconf->threadID);
    CUresult cuResult;

    for(eventmap_cuda_t::iterator it = eventMap->begin(); it != eventMap->end(); ++it) {
        cuResult = cuEventSynchronize(it->second);
        if(cuResult != CUDA_SUCCESS) {
            fprintf(stderr, "[ERROR in CudaDriver::HI_wait_all()] failed wait on CUDA queue %d with error %d (%s)\n", it->first, cuResult, cuda_error_code(cuResult));
			exit(1);
        }
		HI_postponed_free(it->first-2, tconf->threadID);
    	HI_postponed_tempFree(it->first-2, tconf->acc_device_type_var, tconf->threadID);
    }

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait_all()\n");
	}
#endif
}

//[DEBUG] Below implementation is inefficient.
void CudaDriver::HI_wait_all_async(int async, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait_all_async(%d)\n", async);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    eventmap_cuda_t *eventMap = &threadQueueEventMap.at(tconf->threadID);
    CUresult cuResult;

    for(eventmap_cuda_t::iterator it = eventMap->begin(); it != eventMap->end(); ++it) {
        cuResult = cuEventSynchronize(it->second);
        if(cuResult != CUDA_SUCCESS) {
            fprintf(stderr, "[ERROR in CudaDriver::HI_wait_all_async()] failed wait on CUDA queue %d with error %d (%s)\n", it->first, cuResult, cuda_error_code(cuResult));
			exit(1);
        }
		HI_postponed_free(it->first-2, tconf->threadID);
    	HI_postponed_tempFree(it->first-2, tconf->acc_device_type_var, tconf->threadID);
    }

    CUevent event2 = getEvent(async, tconf->threadID);
    cuResult = cuEventSynchronize(event2);

    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_wait_all_async()] failed wait on CUDA queue %d with error %d (%s)\n", async, cuResult, cuda_error_code(cuResult));
		exit(1);
    }
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait_all_async(%d)\n", async);
	}
#endif
}

int CudaDriver::HI_async_test(int asyncId, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_async_test(%d)\n", asyncId);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent(asyncId, tconf->threadID);

    CUresult cuResult = cuEventQuery(event);

    if(cuResult != CUDA_SUCCESS) {
        //fprintf(stderr, "in CudaDriver::HI_async_test()] stream %d code %d\n", asyncId, cuResult);
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_async_test(%d)\n", asyncId);
	}
#endif
        return 0;
    }

    HI_postponed_free(asyncId, tconf->threadID);
    HI_postponed_tempFree(asyncId, tconf->acc_device_type_var, tconf->threadID);
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_async_test(%d)\n", asyncId);
	}
#endif
    return 1;
}

int CudaDriver::HI_async_test_ifpresent(int asyncId, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_async_test_ifpresent(%d)\n", asyncId);
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    CUevent event = getEvent_ifpresent(asyncId, tconf->threadID);
	if( event != NULL ) {

    	CUresult cuResult = cuEventQuery(event);

    	if(cuResult != CUDA_SUCCESS) {
        	//fprintf(stderr, "in CudaDriver::HI_async_test_ifpresent()] stream %d code %d\n", asyncId, cuResult);
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_async_test_ifpresent(%d)\n", asyncId);
	}
#endif
        	return 0;
    	}

    	HI_postponed_free(asyncId, tconf->threadID);
    	HI_postponed_tempFree(asyncId, tconf->acc_device_type_var, tconf->threadID);
	}
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_async_test_ifpresent(%d)\n", asyncId);
	}
#endif
    return 1;
}

int CudaDriver::HI_async_test_all(int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_async_test_all()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
    eventmap_cuda_t *eventMap = &threadQueueEventMap.at(tconf->threadID);
    CUresult cuResult;

    std::set<int> queuesChecked;

    for(eventmap_cuda_t::iterator it = eventMap->begin(); it != eventMap->end(); ++it) {
        cuResult = cuEventQuery(it->second);
        if(cuResult != CUDA_SUCCESS) {
            return 0;
        }
        queuesChecked.insert(it->first);
    }

    //release the waiting frees
    std::set<int>::iterator it;
    for (it=queuesChecked.begin(); it!=queuesChecked.end(); ++it) {
        HI_postponed_free(*it, tconf->threadID);
    	HI_postponed_tempFree(*it, tconf->acc_device_type_var, tconf->threadID);
    }

#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_async_test_all()\n");
	}
#endif
    return 1;
}

void CudaDriver::HI_wait_for_events(int async, int num_waits, int* waits, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_wait_for_events()\n");
	}
#endif
    CUresult cuResult = CUDA_SUCCESS;
    HostConf_t * tconf = getHostConf(threadID);
    if (num_waits > 0 && async != DEFAULT_QUEUE+tconf->asyncID_offset) {
        CUstream stream = getQueue(async, tconf->threadID);
        for (int i = 0; i < num_waits; i++) {
            if (waits[i] == async) continue;
            CUevent event = getEvent(waits[i], tconf->threadID);
            cuResult = cuStreamWaitEvent(stream, event, 0);
            if(cuResult != CUDA_SUCCESS) {
                fprintf(stderr, "[ERROR in CudaDriver::HI_wait_for_events()] failed to call cuStreamWaitEvent %d (%s)\n", cuResult, cuda_error_code(cuResult));
                exit(1);
            }
        }
    }
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_wait_for_events()\n");
	}
#endif
}

void CudaDriver::HI_malloc(void **devPtr, size_t size, HI_MallocKind_t flags, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_malloc()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif
    CUresult cuResult = cuMemAlloc((CUdeviceptr*)devPtr, (size_t) size);
    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_malloc()] failed to malloc on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
		exit(1);
    }
#ifdef _OPENARC_PROFILE_
	tconf->IDMallocCnt++;
	tconf->IDMallocSize += size;
    tconf->CIDMemorySize += size;
	if( tconf->MIDMemorySize < tconf->CIDMemorySize ) {
		tconf->MIDMemorySize = tconf->CIDMemorySize;
	}
    tconf->totalMallocTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_malloc()\n");
	}
#endif
}


void CudaDriver::HI_free(void *devPtr, int threadID) {
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\tenter CudaDriver::HI_free()\n");
	}
#endif
    HostConf_t * tconf = getHostConf(threadID);
#ifdef _OPENARC_PROFILE_
    double ltime = HI_get_localtime();
#endif

    CUresult cuResult = CUDA_SUCCESS;
    void *devPtr2;
	size_t memSize = 0;
//    if( (HI_get_device_address(devPtr, &devPtr2, DEFAULT_QUEUE+tconf->asyncID_offset, tconf->threadID) == HI_error) ||
    if( (HI_get_device_address(devPtr, &devPtr2, NULL, &memSize, DEFAULT_QUEUE+tconf->asyncID_offset, tconf->threadID) == HI_error) ||
		(devPtr != devPtr2) ) {
		//Free device memory if it is not on unified memory.
    	cuResult = cuMemFree((CUdeviceptr)devPtr);
#ifdef _OPENARC_PROFILE_
		tconf->IDFreeCnt++;
		tconf->CIDMemorySize -= memSize;
#endif
	}

    if(cuResult != CUDA_SUCCESS) {
        fprintf(stderr, "[ERROR in CudaDriver::HI_free()] failed to free on CUDA with error %d (%s)\n", cuResult, cuda_error_code(cuResult));
		exit(1);
    }
#ifdef _OPENARC_PROFILE_
    tconf->totalFreeTime += HI_get_localtime() - ltime;
#endif
#ifdef _OPENARC_PROFILE_
	if( HI_openarcrt_verbosity > 2 ) {
		fprintf(stderr, "[OPENARCRT-INFO]\t\texit CudaDriver::HI_free()\n");
	}
#endif
}
