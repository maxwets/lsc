# LSC - Load Shellcode
---

Always loads the shellcode at the same address.
Sometimes the desired address is already committed, in this case just restart the program.
The program also registers a Vectored Exception Handler that prints the error, registers and stack in case an exception occurs.

|Architecture|Address|
|---|---|
|`x64`|`0x100000000`|
|`x86`|`0x1000000`|

## Usage
```powershell
lsc64.exe c:\path\to\shellcode.x64.bin
lsc86.exe c:\path\to\shellcode.x86.bin
```
