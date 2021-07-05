# Async WinInet
AsyncWinInet stands for **Async**hronous **Win**dows **I**nter**net**.
It is a class which implements asynchronous HTTP request calls and supports GET and POST requests, to a URL and data of your choice.

# Overview
AsyncWinInet gives you the ability to send a request, either GET or POST to a URL given, with the data provided. You can also set your
own user agent in case you want to impersonate a browser. The request is sent using the provided functions from the Windows API and a response
is returned in an asynchronous manner (the client will wait for a response, if there is one, you can read it).

# Usage and Tips
You can use this class in cases where you want to send data to a web service or a PHP script and want a non-blocking UI interface. 
The asynchronous technology used will ensure that a response will be waited for and be provided to you when it is received.

**Example GET request:**

```c++
#include "AsyncInet.h"

using namespace std;
int main(void)
{
    AsyncInet inet;

    if (inet.Connect("example.com"))
    {
        if (inet.SendRequest("test.php?id=5", RequestType::GET, "", ""))
        {
            string szResponse;
            char szBuf[1024];
            int iLength;

            while ((iLength = inet.ReadData(reinterpret_cast<PBYTE>(szBuf), 1024)) > 0)
            {

                szBuf[iLength] = 0;
                szResponse += szBuf;
            }

            cout << szResponse.c_str() << endl;
        }
    }


    return 0;
};
```

**Example POST request:**
```c++
#include "AsyncInet.h"

using namespace std;
int main(void)
{
    AsyncInet inet;

    if (inet.Connect("httpbin.org"))
    {
        if (inet.SendRequest("post", RequestType::POST, "somedata=somevalue&anotherone=anothervalue", ""))
        {
            string szResponse;
            char szBuf[1024];
            int iLength;

            while ((iLength = inet.ReadData(reinterpret_cast<PBYTE>(szBuf), 1024)) > 0)
            {
                szBuf[iLength] = 0;
                szResponse += szBuf;
            }

            cout << szResponse.c_str() << endl;
        }
    }


    return 0;
};
```
