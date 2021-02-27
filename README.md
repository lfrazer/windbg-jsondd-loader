# windbg-jsondd-loader
Load x64dbg json debug data in WinDbg

Use the IDA plugin https://github.com/x64dbg/x64dbgida to export symbol data from IDA

Based on the synthetic symbol test plugin made by Aleksey R. (aka EreTIk)
Adds a new command !loadjsondd <PATH> which loads x64dbg json debug data into windbg as synthetic symbols.  Only tested on x64 data!
