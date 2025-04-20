#include "driver.h"
#include "communication.hpp"
  
 
NTSTATUS main() {
    bool UMalive = true;
    const UINT32 pageIndex = KeGetCurrentProcessorIndex();
	printfx("[+] page index: %d\n", pageIndex);

    PEPROCESS target_process = nullptr;
	bool firsttimepid = false;
	int target_pid = 0;
    
    while (UMalive) {
          
      
         
         if(InterlockedCompareExchange(&req->Ready, 0, 1) == 1){
             InterlockedExchange(&req->Ready, 0);

         switch (req->Op) {
        case op_r:
            game::read();  
            break;
        case op_w:
            game::write();
            
            break;
        case op_exit:

            UMalive = false;
            
            break;

        case op_a:
            req->buffer = (PVOID)212;
			target_pid = req->g_pid;
			printfx("[+] target pid: %d\n", target_pid);
            InterlockedExchange(&req->uready, 1);
            
			//usermode::end();
              
             break;
        }
         if (!UMalive) {
             break;
         }
        }
         YieldProcessor(); // Prevent CPU hogging

    }
    usermode::exit();

  

    return STATUS_SUCCESS;
}

NTSTATUS EP() {
    //getting ntos
    NTSTATUS status = STATUS_SUCCESS;
 
   
    printfx("Start Connection\n");
    if (!NT_SUCCESS(usermode::InitializeCommunication())) { return STATUS_ABANDONED; }
	return main();

}
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject); UNREFERENCED_PARAMETER(RegistryPath);
	printfx("Driver loaded\n");
	 
	 

  
    return EP();
}