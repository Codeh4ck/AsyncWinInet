#include "AsyncInet.h"
#pragma comment(lib, "Wininet.lib")

AsyncInet::AsyncInet()
{
    this->hConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    this->hRequestOpenEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    this->hRequestCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    this->hInetInstance = NULL;
    this->hInetConnect = NULL;
    this->hInetRequest = NULL;

    this->context.obj = nullptr;
    this->context.dwContext = NULL;
}

AsyncInet::~AsyncInet()
{
    if (this->hConnectEvent)
        CloseHandle(this->hConnectEvent);

    if (this->hRequestOpenEvent)
        CloseHandle(this->hRequestOpenEvent);

    if (this->hRequestCompleteEvent)
        CloseHandle(this->hRequestCompleteEvent);

    CloseConnection();
}

BOOL AsyncInet::Connect( std::string szAddr, unsigned short port, std::string szAgent, DWORD dwTimeout)
{
    CloseConnection();

    ResetEvent(this->hConnectEvent);
    ResetEvent(this->hRequestOpenEvent);
    ResetEvent(this->hRequestCompleteEvent);
    
    if (!(this->hInetInstance = InternetOpenA(szAgent.c_str(),INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC)))
    {
        return FALSE;
    }

    if (InternetSetStatusCallbackA(this->hInetInstance, (INTERNET_STATUS_CALLBACK)&CallbackFunction))
    {
        return FALSE;
    }

    this->context.dwContext = AsyncInet::CONTEXT_CONNECT;
    this->context.obj = this;

    this->hInetConnect = InternetConnectA(this->hInetInstance, szAddr.c_str(),
        port, NULL, NULL,
        INTERNET_SERVICE_HTTP, INTERNET_FLAG_KEEP_CONNECTION |
        INTERNET_FLAG_NO_CACHE_WRITE, reinterpret_cast<DWORD>(&this->context));

    if (this->hInetConnect == NULL)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject(this->hConnectEvent, dwTimeout))
                return FALSE;
        }
        return FALSE;
    }

    if (this->hInetConnect == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

void AsyncInet::CloseConnection()
{
    if (this->hInetInstance)
        InternetCloseHandle(this->hInetInstance);

    if (this->hInetConnect)
        InternetCloseHandle(this->hInetConnect);

    if (this->hInetRequest)
        InternetCloseHandle(this->hInetRequest);

    this->hInetInstance = NULL;
    this->hInetConnect = NULL;
    this->hInetRequest = NULL;

}

void WINAPI AsyncInet::CallbackFunction(
    HINTERNET hInternet,
    DWORD dwContext,
    DWORD dwInetStatus,
    LPVOID lpStatusInfo,
    DWORD dwStatusInfoLength)
{

    InetContext *ctx = reinterpret_cast<InetContext*>(dwContext);

    switch (ctx->dwContext)
    {
    case AsyncInet::CONTEXT_CONNECT:
        if (dwInetStatus == INTERNET_STATUS_HANDLE_CREATED)
        {
            INTERNET_ASYNC_RESULT *inetResult = reinterpret_cast<INTERNET_ASYNC_RESULT*>(lpStatusInfo);
            ctx->obj->hInetConnect = reinterpret_cast<HINTERNET>(inetResult->dwResult);
            SetEvent(ctx->obj->hConnectEvent);
        }
        break;
    case AsyncInet::CONTEXT_REQUESTHANDLE:
        switch (dwInetStatus)
        {
        case INTERNET_STATUS_HANDLE_CREATED:
            {
                INTERNET_ASYNC_RESULT *inetResult1 = reinterpret_cast<INTERNET_ASYNC_RESULT*>(lpStatusInfo);
                ctx->obj->hInetRequest = reinterpret_cast<HINTERNET>(inetResult1->dwResult);
                SetEvent(ctx->obj->hRequestOpenEvent);
            }
            break;
        case INTERNET_STATUS_REQUEST_COMPLETE:
            SetEvent(ctx->obj->hRequestCompleteEvent);
            break;
        }
        break;
    }
}

BOOL AsyncInet::SendRequest(std::string szURL, RequestType requestType, std::string szRequestData, std::string szReferrer, DWORD dwTimeout)
{

    this->context.dwContext = AsyncInet::CONTEXT_REQUESTHANDLE;
    this->context.obj = this;

    std::string szVerb;
    if (requestType == RequestType::GET)
        szVerb = "GET";
    else
        szVerb = "POST";

    this->hInetRequest = HttpOpenRequestA(this->hInetConnect, szVerb.c_str(), szURL.c_str(),
        NULL, szReferrer.c_str(), NULL,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION
        | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_FORMS_SUBMIT
        | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
        | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP, reinterpret_cast<DWORD>(&this->context));

    if (this->hInetRequest == NULL)
    {
        std::cerr << GetLastError() << std::endl;
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject(this->hRequestOpenEvent, dwTimeout))
                return FALSE;
        }
    }

    if (this->hInetRequest == NULL)
        return FALSE;

    BOOL requestFlag;

    if (requestType == RequestType::GET)
    {
        requestFlag = HttpSendRequestA(this->hInetRequest, NULL, 0, const_cast<char*>(szRequestData.c_str()), szRequestData.size());
    }
    else {
        std::string szHeaders = "Content-Type: application/x-www-form-urlencoded";
        requestFlag = HttpSendRequestA(this->hInetRequest, szHeaders.c_str(), szHeaders.size(), const_cast<char*>(szRequestData.c_str()), szRequestData.size());
    }

    if (!requestFlag)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject(this->hRequestCompleteEvent, dwTimeout) == WAIT_TIMEOUT)
            {
                CloseConnection();
                return FALSE;
            }
        }
    }
    return TRUE;
}

unsigned long AsyncInet::ReadData(PBYTE lpBuffer, DWORD dwSize, DWORD dwTimeout)
{
    INTERNET_BUFFERS inetBuffers;
    memset(&inetBuffers, 0, sizeof(inetBuffers));

    inetBuffers.dwStructSize = sizeof(inetBuffers);
    inetBuffers.lpvBuffer = lpBuffer;
    inetBuffers.dwBufferLength = dwSize - 1;

    this->context.dwContext = AsyncInet::CONTEXT_REQUESTHANDLE;
    this->context.obj = this;

    if (!InternetReadFileEx(this->hInetRequest, &inetBuffers, 0,
        reinterpret_cast<DWORD>(&this->context)))
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject(this->hRequestCompleteEvent, dwTimeout) == WAIT_TIMEOUT)
            {
                return FALSE;
            }
        }
    }

    return inetBuffers.dwBufferLength;
}
