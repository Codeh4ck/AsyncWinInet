typedef struct _InetContext
{
    AsyncInet *obj;
    DWORD dwContext;
} InetContext;

typedef enum _RequestType {
    GET,
    POST
} RequestType;

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
        string szAddr,
        unsigned short port = INTERNET_DEFAULT_HTTP_PORT,
        string szAgent = "AsyncInet",
        DWORD dwTimeout = 20 * 1000);

    BOOL SendRequest(
        string szURL,
        RequestType requestType,
        string szRequestData,
        string szReferrer,
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