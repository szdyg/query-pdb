<p align="center">
    <img src="https://img.shields.io/github/actions/workflow/status/zouxianyu/query-pdb/build_server.yml">
    <img src="https://img.shields.io/github/license/zouxianyu/query-pdb">
</p>

# query-pdb

**query-pdb** is a server-side software for parsing PDB files. The software provides PDB online parsing service, the client only needs to send a request to the server to get the required structures and enumerations, and no longer needs to download the complete PDB file.

![design](rsrc/design.png)



## 关于这个分支

本分支在原分支的基础上做了以下修改，以便用于生产部署

1. 将post请求，改为了get，从而获得cdn，nginx缓存支持，减少重复的cpu，磁盘io消耗。

2. 统一请求格式，服务器直接返回所有字段，简化开发。

3. server 端移除windows支持，完全容器化。

4. 原先的 guid和age 2个参数，现直接合并成了一个。
   
   

## 测试服务器

http://query-pdb.szdyg.cn

https://query-pdb.szdyg.cn



请求参数



| 参数    | 含义                     |
|:----- |:---------------------- |
| pdb   | pdb名称                  |
| guid  | pe文件的guid + age        |
| query | 需要查询的符号或者结构，多个字段用",“拼接 |





### 1. symbol

https://query-pdb.szdyg.cn/symbol?pdb=ntkrnlmp.pdb&guid=8F0F3D677778391600F4EB2301FFC7A51&query=KdpStub,MmAccessFault



```json
{
    "KdpStub": 3773768,
    "MmAccessFault": 2454256
}

```



### 2.struct

https://query-pdb.szdyg.cn/struct?pdb=ntkrnlmp.pdb&guid=8F0F3D677778391600F4EB2301FFC7A51&query=_EPROCESS



```json
{
    "_EPROCESS": {
        "AccountingFolded": {
            "bitfield_offset": 1,
            "offset": 1120
        },
        "ActiveProcessLinks": {
            "bitfield_offset": 0,
            "offset": 1096
        },
        "ActiveThreads": {
            "bitfield_offset": 0,
            "offset": 1520
        },
        "ActiveThreadsHighWatermark": {
            "bitfield_offset": 0,
            "offset": 2136
        },
        "AddressCreationLock": {
            "bitfield_offset": 0,
            "offset": 1224
        },
        "AddressPolicyFrozen": {
            "bitfield_offset": 14,
            "offset": 2172
        },
        "AddressSpaceInitialized": {
            "bitfield_offset": 10,
            "offset": 1124
        },
        "AffinityPermanent": {
            "bitfield_offset": 18,
            "offset": 1120
        },
        "AffinityUpdateEnable": {
            "bitfield_offset": 19,
            "offset": 1120
        },
        "AllowedCpuSets": {
            "bitfield_offset": 0,
            "offset": 2376
        },
        "AllowedCpuSetsIndirect": {
            "bitfield_offset": 0,
            "offset": 2376
        },
        "AlpcContext": {
            "bitfield_offset": 0,
            "offset": 2048
        },
        "AltSyscall": {
            "bitfield_offset": 25,
            "offset": 2172
        },
        "AuxiliaryProcess": {
            "bitfield_offset": 5,
            "offset": 2172
        },
        "BreakOnTermination": {
            "bitfield_offset": 13,
            "offset": 1124
        },
        "CloneRoot": {
            "bitfield_offset": 0,
            "offset": 1264
        },
        "CommitCharge": {
            "bitfield_offset": 0,
            "offset": 1608
        },
        "CommitChargeJob": {
            "bitfield_offset": 0,
            "offset": 1256
        },
        "CommitChargeLimit": {
            "bitfield_offset": 0,
            "offset": 1600
        },
        "CommitChargePeak": {
            "bitfield_offset": 0,
            "offset": 1616
        },
        "CommitFailLogged": {
            "bitfield_offset": 10,
            "offset": 2172
        },
        "Cookie": {
            "bitfield_offset": 0,
            "offset": 1320
        },
        "CoverageSamplerContext": {
            "bitfield_offset": 0,
            "offset": 2544
        },
        "Crashed": {
            "bitfield_offset": 2,
            "offset": 2172
        },
        "CreateInterruptTime": {
            "bitfield_offset": 0,
            "offset": 2304
        },
        "CreateReported": {
            "bitfield_offset": 0,
            "offset": 1124
        },
        "CreateTime": {
            "bitfield_offset": 0,
            "offset": 1128
        },
        "CreateUnbiasedInterruptTime": {
            "bitfield_offset": 0,
            "offset": 2312
        },
        "CrossSessionCreate": {
            "bitfield_offset": 7,
            "offset": 1120
        },
        "DebugPort": {
            "bitfield_offset": 0,
            "offset": 1400
        },
        "DefaultCpuSets": {
            "bitfield_offset": 0,
            "offset": 2384
        },
        "DefaultCpuSetsIndirect": {
            "bitfield_offset": 0,
            "offset": 2384
        },
        "DefaultHardErrorProcessing": {
            "bitfield_offset": 0,
            "offset": 1528
        },
        "DefaultIoPriority": {
            "bitfield_offset": 27,
            "offset": 1124
        },
        "DefaultPagePriority": {
            "bitfield_offset": 12,
            "offset": 1120
        },
        "DeprioritizeViews": {
            "bitfield_offset": 14,
            "offset": 1124
        },
        "DeviceAsid": {
            "bitfield_offset": 0,
            "offset": 2176
        },
        "DeviceMap": {
            "bitfield_offset": 0,
            "offset": 1416
        },
        "DisableSystemAllowedCpuSet": {
            "bitfield_offset": 27,
            "offset": 1120
        },
        "DisabledComponentFlags": {
            "bitfield_offset": 0,
            "offset": 2872
        },
        "DisallowUserTerminate": {
            "bitfield_offset": 27,
            "offset": 2172
        },
        "DiskCounters": {
            "bitfield_offset": 0,
            "offset": 2232
        },
        "DiskIoAttribution": {
            "bitfield_offset": 0,
            "offset": 2392
        },
        "DxgProcess": {
            "bitfield_offset": 0,
            "offset": 2400
        },
        "DynamicEHContinuationTargetsLock": {
            "bitfield_offset": 0,
            "offset": 2848
        },
        "DynamicEHContinuationTargetsTree": {
            "bitfield_offset": 0,
            "offset": 2840
        },
        "DynamicEnforcedCetCompatibleRanges": {
            "bitfield_offset": 0,
            "offset": 2856
        },
        "EmptyJobEvaluated": {
            "bitfield_offset": 11,
            "offset": 1120
        },
        "EnableOptionalXStateFeaturesLock": {
            "bitfield_offset": 0,
            "offset": 2880
        },
        "EnableProcessLocalExecProtectVmLogging": {
            "bitfield_offset": 29,
            "offset": 2172
        },
        "EnableProcessRemoteExecProtectVmLogging": {
            "bitfield_offset": 28,
            "offset": 2172
        },
        "EnableProcessSuspendResumeLogging": {
            "bitfield_offset": 19,
            "offset": 2172
        },
        "EnableReadVmLogging": {
            "bitfield_offset": 24,
            "offset": 1120
        },
        "EnableThreadSuspendResumeLogging": {
            "bitfield_offset": 20,
            "offset": 2172
        },
        "EnableWriteVmLogging": {
            "bitfield_offset": 25,
            "offset": 1120
        },
        "EnclaveLock": {
            "bitfield_offset": 0,
            "offset": 2264
        },
        "EnclaveNumber": {
            "bitfield_offset": 0,
            "offset": 2256
        },
        "EnclaveTable": {
            "bitfield_offset": 0,
            "offset": 2248
        },
        "EnergyContext": {
            "bitfield_offset": 0,
            "offset": 2280
        },
        "EtwDataSource": {
            "bitfield_offset": 0,
            "offset": 1424
        },
        "ExceptionPortData": {
            "bitfield_offset": 0,
            "offset": 1200
        },
        "ExceptionPortState": {
            "bitfield_offset": 0,
            "offset": 1200
        },
        "ExceptionPortValue": {
            "bitfield_offset": 0,
            "offset": 1200
        },
        "ExitProcessReported": {
            "bitfield_offset": 3,
            "offset": 1120
        },
        "ExitStatus": {
            "bitfield_offset": 0,
            "offset": 2004
        },
        "ExitTime": {
            "bitfield_offset": 0,
            "offset": 2112
        },
        "ExplicitAffinity": {
            "bitfield_offset": 21,
            "offset": 1120
        },
        "FailFastOnCommitFail": {
            "bitfield_offset": 8,
            "offset": 1124
        },
        "FatalAccessTerminationRequested": {
            "bitfield_offset": 26,
            "offset": 1120
        },
        "Flags": {
            "bitfield_offset": 0,
            "offset": 1124
        },
        "Flags2": {
            "bitfield_offset": 0,
            "offset": 1120
        },
        "Flags3": {
            "bitfield_offset": 0,
            "offset": 2172
        },
        "ForceWakeCharge": {
            "bitfield_offset": 6,
            "offset": 1120
        },
        "ForegroundExternal": {
            "bitfield_offset": 16,
            "offset": 2172
        },
        "ForegroundSystem": {
            "bitfield_offset": 17,
            "offset": 2172
        },
        "ForkInProgress": {
            "bitfield_offset": 0,
            "offset": 1248
        },
        "GhostCount": {
            "bitfield_offset": 3,
            "offset": 2171
        },
        "HangCount": {
            "bitfield_offset": 0,
            "offset": 2171
        },
        "HasAddressSpace": {
            "bitfield_offset": 18,
            "offset": 1124
        },
        "HideImageBaseAddresses": {
            "bitfield_offset": 13,
            "offset": 2172
        },
        "HighGraphicsPriority": {
            "bitfield_offset": 9,
            "offset": 2172
        },
        "HighMemoryPriority": {
            "bitfield_offset": 18,
            "offset": 2172
        },
        "HighPriorityFaultsAllowed": {
            "bitfield_offset": 0,
            "offset": 2272
        },
        "HighestUserAddress": {
            "bitfield_offset": 0,
            "offset": 1496
        },
        "IdealProcessorAssignmentBlock": {
            "bitfield_offset": 0,
            "offset": 2560
        },
        "ImageFileName": {
            "bitfield_offset": 0,
            "offset": 1448
        },
        "ImageFilePointer": {
            "bitfield_offset": 0,
            "offset": 1440
        },
        "ImageNotifyDone": {
            "bitfield_offset": 22,
            "offset": 1124
        },
        "ImagePathHash": {
            "bitfield_offset": 0,
            "offset": 1524
        },
        "InPrivate": {
            "bitfield_offset": 31,
            "offset": 1120
        },
        "IndirectCpuSets": {
            "bitfield_offset": 7,
            "offset": 2172
        },
        "InheritedFromUniqueProcessId": {
            "bitfield_offset": 0,
            "offset": 1344
        },
        "InvertedFunctionTable": {
            "bitfield_offset": 0,
            "offset": 2120
        },
        "InvertedFunctionTableLock": {
            "bitfield_offset": 0,
            "offset": 2128
        },
        "Job": {
            "bitfield_offset": 0,
            "offset": 1296
        },
        "JobLinks": {
            "bitfield_offset": 0,
            "offset": 1480
        },
        "JobNotReallyActive": {
            "bitfield_offset": 0,
            "offset": 1120
        },
        "JobVadsAreTracked": {
            "bitfield_offset": 3,
            "offset": 2172
        },
        "KTimer2Sets": {
            "bitfield_offset": 0,
            "offset": 2428
        },
        "KTimerSets": {
            "bitfield_offset": 0,
            "offset": 2424
        },
        "LargePrivateVadCount": {
            "bitfield_offset": 0,
            "offset": 2140
        },
        "LastAppState": {
            "bitfield_offset": 61,
            "offset": 2336
        },
        "LastAppStateUpdateTime": {
            "bitfield_offset": 0,
            "offset": 2328
        },
        "LastAppStateUptime": {
            "bitfield_offset": 0,
            "offset": 2336
        },
        "LastFreezeInterruptTime": {
            "bitfield_offset": 0,
            "offset": 2224
        },
        "LastReportMemory": {
            "bitfield_offset": 5,
            "offset": 1120
        },
        "LastThreadExitStatus": {
            "bitfield_offset": 0,
            "offset": 1532
        },
        "LaunchPrefetched": {
            "bitfield_offset": 19,
            "offset": 1124
        },
        "LockedPagesList": {
            "bitfield_offset": 0,
            "offset": 1544
        },
        "Machine": {
            "bitfield_offset": 0,
            "offset": 2412
        },
        "ManageExecutableMemoryWrites": {
            "bitfield_offset": 4,
            "offset": 1124
        },
        "MemoryCompressionProcess": {
            "bitfield_offset": 30,
            "offset": 2172
        },
        "Minimal": {
            "bitfield_offset": 0,
            "offset": 2172
        },
        "MitigationFlags": {
            "bitfield_offset": 0,
            "offset": 2512
        },
        "MitigationFlags2": {
            "bitfield_offset": 0,
            "offset": 2516
        },
        "MitigationFlags2Values": {
            "bitfield_offset": 0,
            "offset": 2516
        },
        "MitigationFlags3": {
            "bitfield_offset": 0,
            "offset": 2928
        },
        "MitigationFlags3Values": {
            "bitfield_offset": 0,
            "offset": 2928
        },
        "MitigationFlagsValues": {
            "bitfield_offset": 0,
            "offset": 2512
        },
        "MmHotPatchContext": {
            "bitfield_offset": 0,
            "offset": 2552
        },
        "MmProcessLinks": {
            "bitfield_offset": 0,
            "offset": 1984
        },
        "MmReserved": {
            "bitfield_offset": 0,
            "offset": 1216
        },
        "ModifiedPageCount": {
            "bitfield_offset": 0,
            "offset": 2000
        },
        "NeedsHandleRundown": {
            "bitfield_offset": 8,
            "offset": 1120
        },
        "NewProcessReported": {
            "bitfield_offset": 2,
            "offset": 1120
        },
        "NoDebugInherit": {
            "bitfield_offset": 1,
            "offset": 1124
        },
        "NumberOfLockedPages": {
            "bitfield_offset": 0,
            "offset": 1280
        },
        "NumberOfPrivatePages": {
            "bitfield_offset": 0,
            "offset": 1272
        },
        "ObjectTable": {
            "bitfield_offset": 0,
            "offset": 1392
        },
        "OtherOperationCount": {
            "bitfield_offset": 0,
            "offset": 1568
        },
        "OtherTransferCount": {
            "bitfield_offset": 0,
            "offset": 1592
        },
        "OutswapEnabled": {
            "bitfield_offset": 6,
            "offset": 1124
        },
        "Outswapped": {
            "bitfield_offset": 7,
            "offset": 1124
        },
        "OverrideAddressSpace": {
            "bitfield_offset": 17,
            "offset": 1124
        },
        "OwnerProcessId": {
            "bitfield_offset": 0,
            "offset": 1352
        },
        "PageCombineSequence": {
            "bitfield_offset": 0,
            "offset": 2876
        },
        "PageDirectoryPte": {
            "bitfield_offset": 0,
            "offset": 1432
        },
        "PageTableCommitmentLock": {
            "bitfield_offset": 0,
            "offset": 1232
        },
        "ParentSecurityDomain": {
            "bitfield_offset": 0,
            "offset": 2536
        },
        "PartitionObject": {
            "bitfield_offset": 0,
            "offset": 2520
        },
        "PathRedirectionHashes": {
            "bitfield_offset": 0,
            "offset": 2888
        },
        "Pcb": {
            "bitfield_offset": 0,
            "offset": 0
        },
        "PdeUpdateNeeded": {
            "bitfield_offset": 23,
            "offset": 1124
        },
        "PeakVirtualSize": {
            "bitfield_offset": 0,
            "offset": 1168
        },
        "Peb": {
            "bitfield_offset": 0,
            "offset": 1360
        },
        "PicoContext": {
            "bitfield_offset": 0,
            "offset": 2240
        },
        "PicoCreated": {
            "bitfield_offset": 10,
            "offset": 1120
        },
        "PrefetchTrace": {
            "bitfield_offset": 0,
            "offset": 1536
        },
        "PrefilterException": {
            "bitfield_offset": 6,
            "offset": 2171
        },
        "PrimaryTokenFrozen": {
            "bitfield_offset": 15,
            "offset": 1120
        },
        "PriorityClass": {
            "bitfield_offset": 0,
            "offset": 1463
        },
        "ProcessDelete": {
            "bitfield_offset": 3,
            "offset": 1124
        },
        "ProcessExecutionState": {
            "bitfield_offset": 22,
            "offset": 1120
        },
        "ProcessExiting": {
            "bitfield_offset": 2,
            "offset": 1124
        },
        "ProcessFirstResume": {
            "bitfield_offset": 15,
            "offset": 2172
        },
        "ProcessInSession": {
            "bitfield_offset": 16,
            "offset": 1124
        },
        "ProcessInserted": {
            "bitfield_offset": 26,
            "offset": 1124
        },
        "ProcessLock": {
            "bitfield_offset": 0,
            "offset": 1080
        },
        "ProcessQuotaPeak": {
            "bitfield_offset": 0,
            "offset": 1152
        },
        "ProcessQuotaUsage": {
            "bitfield_offset": 0,
            "offset": 1136
        },
        "ProcessRundown": {
            "bitfield_offset": 25,
            "offset": 1124
        },
        "ProcessSelfDelete": {
            "bitfield_offset": 30,
            "offset": 1124
        },
        "ProcessStateChangeInProgress": {
            "bitfield_offset": 30,
            "offset": 1120
        },
        "ProcessStateChangeRequest": {
            "bitfield_offset": 28,
            "offset": 1120
        },
        "ProcessTimerDelay": {
            "bitfield_offset": 0,
            "offset": 2416
        },
        "ProcessVerifierTarget": {
            "bitfield_offset": 16,
            "offset": 1120
        },
        "PropagateNode": {
            "bitfield_offset": 20,
            "offset": 1120
        },
        "Protection": {
            "bitfield_offset": 0,
            "offset": 2170
        },
        "QuotaBlock": {
            "bitfield_offset": 0,
            "offset": 1384
        },
        "ReadOperationCount": {
            "bitfield_offset": 0,
            "offset": 1552
        },
        "ReadTransferCount": {
            "bitfield_offset": 0,
            "offset": 1576
        },
        "RefTraceEnabled": {
            "bitfield_offset": 9,
            "offset": 1120
        },
        "RelinquishedCommit": {
            "bitfield_offset": 8,
            "offset": 2172
        },
        "ReplacingPageRoot": {
            "bitfield_offset": 1,
            "offset": 2172
        },
        "ReportCommitChanges": {
            "bitfield_offset": 4,
            "offset": 1120
        },
        "RequestedTimerResolution": {
            "bitfield_offset": 0,
            "offset": 2104
        },
        "ReserveFailLogged": {
            "bitfield_offset": 11,
            "offset": 2172
        },
        "Reserved": {
            "bitfield_offset": 20,
            "offset": 1124
        },
        "RestrictSetThreadContext": {
            "bitfield_offset": 17,
            "offset": 1120
        },
        "RotateInProgress": {
            "bitfield_offset": 0,
            "offset": 1240
        },
        "RundownProtect": {
            "bitfield_offset": 0,
            "offset": 1112
        },
        "SeAuditProcessCreationInfo": {
            "bitfield_offset": 0,
            "offset": 1472
        },
        "SectionBaseAddress": {
            "bitfield_offset": 0,
            "offset": 1312
        },
        "SectionObject": {
            "bitfield_offset": 0,
            "offset": 1304
        },
        "SectionSignatureLevel": {
            "bitfield_offset": 0,
            "offset": 2169
        },
        "SecurityDomain": {
            "bitfield_offset": 0,
            "offset": 2528
        },
        "SecurityDomainChanged": {
            "bitfield_offset": 21,
            "offset": 2172
        },
        "SecurityFreezeComplete": {
            "bitfield_offset": 22,
            "offset": 2172
        },
        "SecurityPort": {
            "bitfield_offset": 0,
            "offset": 1464
        },
        "SequenceNumber": {
            "bitfield_offset": 0,
            "offset": 2296
        },
        "ServerSilo": {
            "bitfield_offset": 0,
            "offset": 2160
        },
        "Session": {
            "bitfield_offset": 0,
            "offset": 1368
        },
        "SessionProcessLinks": {
            "bitfield_offset": 0,
            "offset": 1184
        },
        "SetTimerResolution": {
            "bitfield_offset": 12,
            "offset": 1124
        },
        "SetTimerResolutionLink": {
            "bitfield_offset": 31,
            "offset": 1124
        },
        "SharedCommitCharge": {
            "bitfield_offset": 0,
            "offset": 2344
        },
        "SharedCommitLinks": {
            "bitfield_offset": 0,
            "offset": 2360
        },
        "SharedCommitLock": {
            "bitfield_offset": 0,
            "offset": 2352
        },
        "SignatureLevel": {
            "bitfield_offset": 0,
            "offset": 2168
        },
        "SmallestTimerResolution": {
            "bitfield_offset": 0,
            "offset": 2108
        },
        "Spare0": {
            "bitfield_offset": 0,
            "offset": 2414
        },
        "Spare1": {
            "bitfield_offset": 0,
            "offset": 1376
        },
        "SubsystemProcess": {
            "bitfield_offset": 6,
            "offset": 2172
        },
        "SvmData": {
            "bitfield_offset": 0,
            "offset": 2184
        },
        "SvmLock": {
            "bitfield_offset": 0,
            "offset": 2200
        },
        "SvmProcessDeviceListHead": {
            "bitfield_offset": 0,
            "offset": 2208
        },
        "SvmProcessLock": {
            "bitfield_offset": 0,
            "offset": 2192
        },
        "SyscallProvider": {
            "bitfield_offset": 0,
            "offset": 2896
        },
        "SyscallProviderDispatchContext": {
            "bitfield_offset": 0,
            "offset": 2920
        },
        "SyscallProviderProcessLinks": {
            "bitfield_offset": 0,
            "offset": 2904
        },
        "SystemProcess": {
            "bitfield_offset": 12,
            "offset": 2172
        },
        "ThreadListHead": {
            "bitfield_offset": 0,
            "offset": 1504
        },
        "ThreadListLock": {
            "bitfield_offset": 0,
            "offset": 2144
        },
        "ThreadTimerSets": {
            "bitfield_offset": 0,
            "offset": 2432
        },
        "TimerResolutionIgnore": {
            "bitfield_offset": 26,
            "offset": 2172
        },
        "TimerResolutionLink": {
            "bitfield_offset": 0,
            "offset": 2080
        },
        "TimerResolutionStackRecord": {
            "bitfield_offset": 0,
            "offset": 2096
        },
        "Token": {
            "bitfield_offset": 0,
            "offset": 1208
        },
        "TotalUnbiasedFrozenTime": {
            "bitfield_offset": 0,
            "offset": 2320
        },
        "UniqueProcessId": {
            "bitfield_offset": 0,
            "offset": 1088
        },
        "VadCount": {
            "bitfield_offset": 0,
            "offset": 2024
        },
        "VadHint": {
            "bitfield_offset": 0,
            "offset": 2016
        },
        "VadPhysicalPages": {
            "bitfield_offset": 0,
            "offset": 2032
        },
        "VadPhysicalPagesLimit": {
            "bitfield_offset": 0,
            "offset": 2040
        },
        "VadRoot": {
            "bitfield_offset": 0,
            "offset": 2008
        },
        "VadTrackingDisabled": {
            "bitfield_offset": 4,
            "offset": 2172
        },
        "VdmAllowed": {
            "bitfield_offset": 24,
            "offset": 1124
        },
        "VirtualSize": {
            "bitfield_offset": 0,
            "offset": 1176
        },
        "VirtualTimerListHead": {
            "bitfield_offset": 0,
            "offset": 2448
        },
        "VirtualTimerListLock": {
            "bitfield_offset": 0,
            "offset": 2440
        },
        "Vm": {
            "bitfield_offset": 0,
            "offset": 1664
        },
        "VmContext": {
            "bitfield_offset": 0,
            "offset": 2288
        },
        "VmDeleted": {
            "bitfield_offset": 5,
            "offset": 1124
        },
        "VmProcessorHost": {
            "bitfield_offset": 23,
            "offset": 2172
        },
        "VmProcessorHostTransition": {
            "bitfield_offset": 24,
            "offset": 2172
        },
        "VmTopDown": {
            "bitfield_offset": 21,
            "offset": 1124
        },
        "WakeChannel": {
            "bitfield_offset": 0,
            "offset": 2464
        },
        "WakeInfo": {
            "bitfield_offset": 0,
            "offset": 2464
        },
        "Win32KFilterSet": {
            "bitfield_offset": 0,
            "offset": 2408
        },
        "Win32Process": {
            "bitfield_offset": 0,
            "offset": 1288
        },
        "Win32WindowStation": {
            "bitfield_offset": 0,
            "offset": 1336
        },
        "WnfContext": {
            "bitfield_offset": 0,
            "offset": 2152
        },
        "WoW64Process": {
            "bitfield_offset": 0,
            "offset": 1408
        },
        "WorkingSetWatch": {
            "bitfield_offset": 0,
            "offset": 1328
        },
        "Wow64VaSpace4Gb": {
            "bitfield_offset": 9,
            "offset": 1124
        },
        "WriteOperationCount": {
            "bitfield_offset": 0,
            "offset": 1560
        },
        "WriteTransferCount": {
            "bitfield_offset": 0,
            "offset": 1584
        },
        "WriteWatch": {
            "bitfield_offset": 15,
            "offset": 1124
        }
    }
}

```



### 3. enum

https://query-pdb.szdyg.cn/enum?pdb=ntkrnlmp.pdb&guid=8F0F3D677778391600F4EB2301FFC7A51&query=_OBJECT_INFORMATION_CLASS



```json
{
    "_OBJECT_INFORMATION_CLASS": {
        "MaxObjectInfoClass": 7,
        "ObjectBasicInformation": 0,
        "ObjectHandleFlagInformation": 4,
        "ObjectNameInformation": 1,
        "ObjectSessionInformation": 5,
        "ObjectSessionObjectInformation": 6,
        "ObjectTypeInformation": 2,
        "ObjectTypesInformation": 3
    }
}

```


