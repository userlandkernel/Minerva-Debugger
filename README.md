# Minerva-Debugger
Providing a great interface to kernel, hardware, threads and processes in a great research environment. (WIP)

## Support
- Any jailbreak with HSP4 exported same way as unc0ver.  
- Pre-A12 devices, A12 support coming soon
- At this time only iPhone 6S on 12.0.1 has offsets.

## How To use
1. Jailbreak exporting HSP4.  
2. Attach iPhone via Serial Cable (Alex DCSD), for now but not for long.
3. Run nanokdp or termz to see serial output at default baudrate (115200).  
4. Compile and run app.  

## But it does not work?
1. If it doesn't work mostlikely you are not jailbroken.  
2. If you are sure to be jailbroken, also check whether hsp4 is exported.  
3. If both jailbroken and hsp4 are exported, then are your offsets in the project yet? Rightnow it's only N71AP (iPhone8,1) on 12.0.1  
4. All above is the case but the device panics? Well at this time A12 is not supported yet.  

## Contributing offsets
1. They must conform to the structures in offsets.c / offsets.h.  
2. Don't make a pull request for offsets, it will be ignored. Instead file an issue please.  
3. Each person adding valid and correct offsets deserves credit.  
4. A script for generating a struct of (most) symbol offsets from a kernel will be posted here soon. Built w/ python, radare2 and awk.  
5. Most patch offsets need to be found manually. See my unstripped kexts repository and use [Diaphora](https://diaphora.re) to symbolicate the kernel. Use [XNU source code](https://github.com/UKERN-developers/darwin-xnu) for importing local types into IDA.  

## Contributing to code & Documentation
- See [Documentation](documentation)
- See https://github.com/users/userlandkernel/projects/1/ for SCRUM board

## Credits
- Jake James, for JelbrekLib
- Apple Inc, for XNU. Credit where credit is due right ;)
- Luca Todesco, some of the primitives in the project can be found in his jailbreaks :)
