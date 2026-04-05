#include "pch.h"
#include <windows.h>
#include <ctime>
#include <cstring>
#include <stdio.h>

const DWORD_PTR TARGET_OFFSET_20260203_0 = 0x88AC18; 
const DWORD_PTR TARGET_OFFSET_20260203_1 = 0x88AC50; 


void GetCurrentDateString(char* buffer, int bufferSize) {
    time_t now = time(0);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        CreateThread(NULL, 0, [](LPVOID) -> DWORD {
            // 获取模块基址
            HMODULE hSoundVoltex = GetModuleHandleA("soundvoltex.dll");
            if (hSoundVoltex == NULL) {
                MessageBoxA(NULL, "无法获取 soundvoltex.dll 模块句柄", "错误", MB_OK);
                return 0;
            }

            // 计算目标地址
            char* targetAddress0 = (char*)hSoundVoltex + TARGET_OFFSET_20260203_0;
            char* targetAddress1 = (char*)hSoundVoltex + TARGET_OFFSET_20260203_1;

            // 保存原始页面保护属性
            DWORD oldProtect = 0;

            while (true) {
                char dateStr[21] = { 0 };
                GetCurrentDateString(dateStr, sizeof(dateStr));

                // 修改页面保护为可读写
                if (VirtualProtect(targetAddress1, sizeof(dateStr), PAGE_READWRITE, &oldProtect)) {
                    SIZE_T bytesWritten = 0;

                    BOOL result0 = WriteProcessMemory(GetCurrentProcess(), targetAddress0, dateStr, sizeof(dateStr), &bytesWritten);
                    BOOL result1 = WriteProcessMemory(GetCurrentProcess(), targetAddress1, dateStr, sizeof(dateStr), &bytesWritten);

                    // 恢复原始保护（可选，但推荐）
                    VirtualProtect(targetAddress0, sizeof(dateStr), oldProtect, &oldProtect);
                    VirtualProtect(targetAddress1, sizeof(dateStr), oldProtect, &oldProtect);
                }
                else {
                    // VirtualProtect 失败
                    static bool first = true;
                    if (first) {
                        char msg[256];
                        sprintf_s(msg, sizeof(msg), "VirtualProtect 失败，错误码: %d", GetLastError());
                        MessageBoxA(NULL, msg, "错误", MB_OK);
                        first = false;
                    }
                }

                Sleep(1000);
            }
            return 0;
            }, NULL, 0, NULL);
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}