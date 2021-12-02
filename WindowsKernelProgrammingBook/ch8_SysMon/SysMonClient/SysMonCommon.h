#pragma once

enum class ItemType :short {
	None,
	ProcessCreate,
	ProcessExit,
	ThreadCreate,
	ThreadExit,
	ImageLoad
};

// 记录事件公有信息
struct ItemHeader {
	ItemType Type;		// 事件类型
	USHORT Size;		// 事件大小(用户能看到的大小)
	LARGE_INTEGER Time;	// 事件时间
};

//------用户模式能看到的数据信息--------
// 退出进程事件信息
struct ProcessExitInfo :ItemHeader {
	ULONG ProcessId;
};

// 创建进程事件信息
struct ProcessCreateInfo :ItemHeader {
	ULONG ProcessId;			// 进程ID
	ULONG ParentProcessId;		// 父进程ID
	USHORT CommandLineLength;	// 命令行长度
	USHORT CommandLineOffset;	// 命令行所在偏移量
	USHORT ImageFileNameLength;	// 映像名长度
	USHORT ImageFileNameOffset;	// 映像名所在偏移量
};

// 创建退出线程事件信息
struct ThreadCreateExitInfo :ItemHeader {
	ULONG ThreadId;
	ULONG ProcessId;
	USHORT ProcessImageFileNameLength;	// 映像名长度(uchar)
	USHORT ProcessImageFileNameOffset;	// 映像名所在偏移量(uchar)
};

// 映像加载事件信息
struct ImageLoadInfo :ItemHeader {
	ULONG ProcessId;
	ULONG ImageBase;
	USHORT ImageLoadPathLength;
	USHORT ImageLoadPathOffset;
};