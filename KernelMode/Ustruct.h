#include "driver.h"

struct commAddr {
    uintptr_t pFct; // int
    uintptr_t pPid; // int
    uintptr_t pAddr; // uintptr_t
    uintptr_t pSize; // size_t
    uintptr_t pValue; // ULONGLONG
    uintptr_t pKM; // response
    uintptr_t pEnd; // bool
};

struct respAddr {
    uintptr_t pBuff; // PVOID
    uintptr_t pStatus; // int
    uintptr_t pEnd; // bool
};

commAddr cAddr;
respAddr rAddr;


struct response {
    PVOID buffer;
    int status;
    bool end;
};

struct communication
{
    int fct; // 10 = Read process memory, 20 = Write process memory
    int pid;
    uintptr_t addr;
    SIZE_T size;
    ULONGLONG value;
    response KMresponse;
    bool end;
};

communication comm;

typedef enum _req_op {
    op_r = 1,
    op_w = 2,
    op_a = 3,
	op_idle = 0,
	op_exit = 420
} req_op;
//  req instance that will be use in the que 
typedef struct _req_Obj {
    volatile LONG Ready;       // 1 = request pending, 0 = request done
    volatile LONG uready;       // 1 = request pending, 0 = request done
    volatile LONG InUse;   // 0=free, 1=processing, 2=done
    int g_pid;
    req_op Op;
    PVOID TargetAddress;
    ULONG Size;
    PVOID buffer;
    NTSTATUS Status;
} Requests, * PreqEntry;
typedef struct _HolyQue {

    volatile LONG producer; // basicly the usermode
    volatile LONG consumer; //kernel 
} holyque, * Pholyque;
typedef struct _HolyQue holyque;

typedef struct _req_Obj Requests;