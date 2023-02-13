#pragma once

enum TssSdkRequestCmdID
{
    TssSdkCmd_GetSdkAntiData = 1,
    TssSdkCmd_RegistSdkEventListener = 2,
    TssSdkCmd_IsAppOnForeground = 3,
    TssSdkCmd_GameScreenshot= 4,
    TssSdkCmd_InvokeCrashFromShell = 5,
    TssSdkCmd_GetCommLibValueByKey = 6,
    TssSdkCmd_GetShellDyMagicCode = 7,
    TssSdkCmd_AddMTCJTask = 8,
    TssSdkCmd_GameScreenshot2 = 9,
    TssSdkCmd_IsEmulator = 10,
    TssSdkCmd_SetToken = 11,
    TssSdkCmd_GenSessionData = 12,
    TssSdkCmd_WaitVerify = 13,
    TssSdkCmd_QueryTssSdkVer = 14,
    TssSdkCmd_EnableDisableItem = 15,
    TssSdkCmd_QueryOpts = 16,
    TssSdkCmd_ReInitMrpcs = 17,
    TssSDKCmd_CommQuery = 18,
    TssSDKCmd_OpenWBSession = 19,			//参数param为TssWinBrigeTask的指针,结果为WinBridge Session的指针, 将写入param的session成员
    TssSDKCmd_SendWBCmd = 20,				//参数param为TssWinBrigeTask的指针,其中param的session成员为WinBridge Session的指针, cmd为要发送的命令, 调用结果将写入param的response成员
    TssSDKCmd_ReleaseWBStrPool = 21,		//参数param为TssWinBrigeTask的指针,其中param的session成员为WinBridge Session的指针
    TssSDKCmd_CloseWBSession = 22,			//参数param为TssWinBrigeTask的指针,其中param的session成员为WinBridge Session的指针
    TssSDKCmd_GetUserTag = 23,
    TssSDKCmd_QueryTssLibcAddr = 24,
    TssSDKCmd_RegistLibcSendListener = 25,
    TssSDKCmd_RegistLibcRecvListener = 26,
    TssSDKCmd_RegistLibcConnectListener = 27,
    TssSDKCmd_GetMrpcsData2Ptr = 28,
    TssSDKCmd_GetTPChannelVer = 29,
    TssSDKCmd_RecvSecSignature = 34,
    TssSdkCmd_HitVerify = 37,
    TssSDKCmd_InitSwitchStr = 45,
};

class TencentSafe {
public:
    static void init();
};

// Hook预定义模版
#define HOOK_DEF(ret, func, ...) \
    ret (*orig_##func)(__VA_ARGS__); \
    ret new_##func(__VA_ARGS__)