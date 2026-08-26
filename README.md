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
        "ActiveProcessLinks": {
            "bitfield_offset": 0,
            "offset": 1096,
            "type": "_LIST_ENTRY"
        },
        "CreateReported": {
            "bitfield_offset": 0,
            "offset": 1124,
            "type": "ULONG"
        },
        "CreateTime": {
            "bitfield_offset": 0,
            "offset": 1128,
            "type": "_LARGE_INTEGER"
        },
        "DefaultIoPriority": {
            "bitfield_offset": 27,
            "offset": 1124,
            "type": "ULONG"
        },
        "ImageFileName": {
            "bitfield_offset": 0,
            "offset": 1448,
            "type": "UCHAR"
        },
        "MitigationFlagsValues": {
            "bitfield_offset": 0,
            "offset": 2512,
            "type": "<unnamed-tag>"
        },
        "Pcb": {
            "bitfield_offset": 0,
            "offset": 0,
            "type": "_KPROCESS"
        },
        "Peb": {
            "bitfield_offset": 0,
            "offset": 1360,
            "type": "_PEB*"
        },
        "Token": {
            "bitfield_offset": 0,
            "offset": 1208,
            "type": "_EX_FAST_REF"
        },
        "UniqueProcessId": {
            "bitfield_offset": 0,
            "offset": 1088,
            "type": "PVOID"
        }
    }
}
```

（上面只摘录了部分字段，实际会返回结构体的全部成员）



每个成员包含以下字段

| 字段              | 含义                                       |
|:--------------- |:---------------------------------------- |
| offset          | 成员相对于结构体起始处的字节偏移                         |
| type            | 成员类型名，指针会按层级追加 `*`，无法解析时为空字符串            |
| bitfield_offset | 位域成员在其存储单元中的起始位；非位域成员为 0                 |



关于 type 的几点说明

1. 数组只返回元素类型，不含长度。例如 `ImageFileName` 实际是 `UCHAR[15]`，这里只给出 `UCHAR`。

2. 指针的写法并不统一：指向自定义类型时为 `_PEB*`，而指向基础类型时可能返回 CodeView 内置的别名，例如 `ULONGLONG*` 会返回 `PULONGLONG`。

3. 匿名结构体或联合返回 `<unnamed-tag>`。



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


