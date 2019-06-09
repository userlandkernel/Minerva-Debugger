# Documentation of The Minerva Debugger

## Concept
The minerva debugger is unlike any debugger, one that opens an interface to XNU kernel phys/virt memory and hardware operations.  
It operates on the main logic that all can be halted / interrupted, inspected and altered and resumed.  
Aside that it serves as an easy human interface to more complex operations as its goal is to bootstrap all functionality in a REPL interface.  

## Requirements
- A jailbroken device with HSP4, I hope an RFC will ever be released for the way Unc0ver has implemented hsp4, it will make life easier.  
- Kernel offsets matching your device, you can check offsets.c for those.  
- An iOS device without Pointer Authentication Codes / Control Flow Integrity.  Preferably one without AMCC Memory Protection.   

## Programming styleguide
- The debugger must be written in C, bridges from Objective-C are allowed but must prevent ARC from causing Use After Free vulnerabilities.  Swift is not preferred.  
- Methods and variables are preferred to be written in camel-case style.  
- Code must be forseen with good comments where complex logic is performed.  
- Stack memory is preferred over heap when possible.  

## Kernel operation programming
- Offsets of symbols and constants in the kernel must be added in offsets.c accordingly.
- JelbrekLib by Jake James is mandatory for accessing Kernel Memory and Executing functions, other primitives must be discussed here by filing an issue (e.g: KPP bypass w/ rwx memory access rights or larger kernel execute gadget by for example JOP programming).  
- WriteAnywhere and ReadAnywhere functions are preferred for small memory operations, for big ones there are copyin and copyout gadgets as seen in Luca Todesco's Yalu for 10.2.  
- Beware of the mighty KTRR (Kernel Text Read-only Region), we can patch most of the kernel's data but not all.  
- Beware that KTRR prevents execution of (most, but not all) system instructions.  
- WatchTower (KPP) runs feel free to defeat or race it on devices prior to AMCC memory protection mitigations.  
- When done with all kernel execution, make sure the kernel terminate functions are called, make sure the function in which terminating is not MIG generated otherwise beware of the ownership rules.  *Cough* *Cough*.  
- You can write wrappers around Kernel_Execute() and use copyout() and Kernel_alloc() for userland to kernelspace copying of arguments.  
- Beware that you cannot use any userland memory addresses / pointers in these wrappers, unless you handle the copying to kernelspace correctly in the function.  

## Threat operation programming
- Hold my beer, we have remote thread operations. Thanks to Ian Beer, from Google Project Zero.  That guy is a true hero.  
- machine_thread gadgets are meant for kernel threads, userland threads can simply use threadports with sendrights and use thread_get_state() and thread_set_state().  

## Serial output
- UART can be enabled and there are wrappers for dealing with serial output.  
- Requires an Alex DCSD cable for accessing the log.  
- Baudrate can be patched. Hooray :)

## Serial input
- Serial Keyboard, but its broken.  

## The video console
- Works only when springboard is killed and screen is not used by any other process.  
- Might require theos tweaks for achieving this by hooking shutdown of springboard. (Slide to poweroff).  

## Donations
- Always welcome but never mandatory, good contributions to the code and project mean a lot too.
- Features are a currency, bugs a burden.  
