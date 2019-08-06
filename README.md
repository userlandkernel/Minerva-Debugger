# Minerva Debugger (WIP)
***TLDR***:  
*Providing a great interface to kernel, hardware, threads and processes in a productive research environment.*  

- Built on top of features of iOS Operating System.  
- Allows interacting with components of the OS like (Mach, IOKit, BSD).  
- Allows interacting with interfaces such as Network, Bluetooth, Serial.  
- Allows interacting with execution of processes, tasks, threads, exception handlers and likewise.  
- Consists of multiple components under more: a local and tcp bound shell, serial I/O and an api server.  
- API has routes for many logical operations in an easy structure as seen in the example:  
Get the task of a process by its pid (task_for_pid()) via https://minerva.local:666/process/{pid}/task
which returns the task information for the given pid including the taskport.  


## Support
- Any jailbreak with HSP4 exported same way as unc0ver.  
- Pre-A12 devices, A12 support coming soon.  
- At this time only iPhone 6S on 12.0.1 has offsets.  


## Architecture
- Designed as a LaunchDaemon
- Built on top of export kernel taskport via HSP4.  
- Is loaded after a jailbreak, e.g: Unc0ver.  
- Will run in the background and entitle itself when needed (e.g: get_task-allow)
- Will enforce authorization by password, stored salted and hashed in the NVRAM defaulting to 'alpine'.  
- Resetting a password requires user to delete NVRAM entry manually.  
- Changing password via Minerva can only be done provided a valid session token + old password.  
- Minerva will always notify the user via alerts on SpringBoard whenever it's password is changed.  
- Will enforce authentication through session tokens that are enforced on any exposed operation for any service (HTTP / local mach).  
- Minerva will work in userland when installed as a regular app instead of a launchdaemon.  
- Minerva requires the Nework Extension for the bound shell, which requires it to be codesigned using an Apple Developer ($100) account.  
- Minerva can work without the bound shell as well, but is pretty limited.  
- Minerva currently has no support planned for a connect-back like shell due to performance and security considerations that require design changes.

## Core Features
- Minerva is meant to expose the iOS Operating system into easy accessible higher level environments, like an API and commandline environment.  
- Minerva is meant to provide the ability to inspect and alter runtime execution of processes, tasks, threads and exception handlers.  
- Minerva is meant to bridge hardware driver access into an easy manageble interface.  
- Minerva is meant to be rather fully featured, productive and useful than secure.  
- Minerva is meant to be fully based on the capabilities of iOS, powered by Mach.  
- Minerva is capable of managing and controlling memory of anything on the system, as far as permissions by hardware and vendor will allow (Read: KPP & KTRR).  
- Minerva is capable of injecting execution through the creation of threads in any task running on the system.  
- Minerva may be capable of injecting (constrainted) instructions into the physical cpu by using ARMv8 CoreSight.  
- Minerva can be managed through a webapp if one creates one on top of the API exposed by the HTTP Server.  
- Minerva can log verbose information to the serial console over UART when provided with a DCSD (e.g. Alex DCSD) cable.  
- Minerva can deal with debug interfaces such as dtrace via pre-bootstrapped third-party modules (kdv by Jonathan Levin).  
- Minerva can load and execute script files in a syntax yet to be designed (Either python, lua, basic or my very own simplescript).  


## KPP and KTRR devices
- KTRR and KPP prevent the kernel from being patched and memory protections (access rights) from being altered making some patches to the kernel impossible.  
- Minerva will not promise to feature a fully implemented defeat of KPP by altering the first level entry pagetables (As seen in Yalu102 by luca todesco), but aim to work towards it.  
- Minerva will not promise to feature a fully implemented defeat of KTRR and Memory Protection by turning off the MMU through injected instructions via Core Sight debugging registers, but will attempt to research and expand on this possibility.  
- Minerva will not promise to feature a partially implemented defeat of KTRR by swapping the TGSZ values in the TCR register on iOS below 12.2 via instructions outside of the protected instructions segment of the kernel text read only region in order to enlarge the granule size of pages to alter the memory access rights but aim to work towards it.  


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


## OpenSource, License exceptions and Contributions
- As minerva is OpenSource under the GNU Affero General Public License v3.0 it is possible to reuse the code in any project if given appropriate credit.  
- As an exception and addition to the GNU Affero General Public License v3.0 the source code of this project and or code using this project must be published on GitHub specifically, and must also be published OpenSource and applications using more than 50% of the code of this project must give credit visible to the user.  
- Contributions to Minerva are much appreciated.  
- Luca Todesco will get a gram of dutch Amnesia Haze donated by me (@userlandkernel) if he manages to implement the KPP bypass in Minerva for devices running iOS before and including 12.1 to be picked up in Amsterdam as guidelined by Dutch law.  
- See [Documentation](documentation) if you want to contribute to development or documentation.  
- See https://github.com/users/userlandkernel/projects/1/ for SCRUM/AGILE board.  


## Contributing offsets
1. They must conform to the structures in offsets.c / offsets.h.  
2. Don't make a pull request for offsets, it will be ignored. Instead file an issue please.  
3. Each person adding valid and correct offsets deserves credit.  
4. A script for generating a struct of (most) symbol offsets from a kernel will be posted here soon. Built w/ python, radare2 and awk.  
5. Most patch offsets need to be found manually. See my unstripped kexts repository and use [Diaphora](http://diaphora.re) to symbolicate the kernel. Use [XNU source code](https://github.com/UKERN-developers/darwin-xnu) for importing local types into IDA.  


## Research done for MinervaDebugger
- MinervaDebugger is the result of many hours of research done currently by one individual that spent most free time on reverse engineering and symbolicating XNU, learning about the iOS Operating System and programming C for practice. The number of hours put into this has way long exceeded 500 hours.  
- MinervaDebugger is meant to be free of costs and used freely by people who will fight for freedom, freedom of speech, freedom of modification of their personal properties and freedom of development in any thinkable way including but not limited to hacking for fun without harm to others or other people's property.  


## Credits
- Jake James, for JelbrekLib
- Apple Inc, for XNU. Credit where credit is due right ;)
- Luca Todesco, some of the primitives in the project can be found in his jailbreaks :)
