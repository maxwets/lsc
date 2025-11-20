# LSC
---

Simple shellcode loader.
Always loads the shellcode at the same address:

|Architecture|Address|
|---|---|
|`x64`|`0x150000000`|
|`x86`|`0x1400000`|

## Usage
```powershell
lsc64.exe c:\path\to\shellcode.x64.bin
lsc86.exe c:\path\to\shellcode.x86.bin
```
