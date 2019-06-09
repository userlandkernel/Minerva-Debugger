# Documentation of The Minerva Debugger

## Concept
The minerva debugger is unlike any debugger, one that opens an interface to XNU kernel phys/virt memory and hardware operations.  
It operates on the main logic that all can be halted / interrupted, inspected and altered and resumed.  
Aside that it serves as an easy human interface to more complex operations as its goal is to bootstrap all functionality in a REPL interface.  

## Requirements
- A jailbroken device with HSP4, I hope an RFC will ever be released for the way Unc0ver has implemented hsp4, it will make life easier.  
- Kernel offsets matching your device, you can check offsets.c for those.  
- An iPhone without Pointer Authentication Codes / Control Flow Integrity.  Preferably one without AMCC Memory Protection.   
