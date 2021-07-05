#include <Windows.h>
#include <WinInet.h>
#include <string>
#include <stdio.h>
#include <iostream>

#pragma execution_character_set( "utf-8" ) 

#define WIN32_LEAN_AND_MEAN
#pragma once

class AsyncInet;

struct InetContext
{
    AsyncInet* obj;
    DWORD dwContext;
};

enum class RequestType {
    GET,
    POST
};

class AsyncInet
{
public:
    AsyncInet();
    ~AsyncInet();

    enum {
        CONTEXT_CONNECT,
        CONTEXT_REQUESTHANDLE
    };

    BOOL Connect(
        std::string szAddr,
        unsigned short port = INTERNET_DEFAULT_HTTP_PORT,
        std::string szAgent = "AsyncInet",
        DWORD dwTimeout = 20 * 1000);

    BOOL SendRequest(
        std::string szURL,
        RequestType requestType,
        std::string szRequestData,
        std::string szReferrer,
        DWORD dwTimeout = 20 * 1000);

    unsigned long ReadData(
        PBYTE lpBuffer,
        DWORD dwSize,
        DWORD dwTimeout = 20 * 1000);

    void CloseConnection();

    static void WINAPI CallbackFunction(
        HINTERNET hInternet,
        DWORD dwContext,
        DWORD dwInetStatus,
        LPVOID lpStatusInfo,
        DWORD dwStatusInfoLength);

protected:
    InetContext context;
    HANDLE hConnectEvent, hRequestOpenEvent, hRequestCompleteEvent;
    HINTERNET hInetInstance, hInetConnect, hInetRequest;

};
