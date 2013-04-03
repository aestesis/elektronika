#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <stdio.h>
#include "md5.h"

# pragma comment(lib, "wbemuuid.lib")

static bool get(IWbemServices *pSvc, BSTR request, LPCWSTR prop, VARIANT *ret)
{
    IEnumWbemClassObject	*pEnumerator = NULL;
    HRESULT					hres;
	bool					b=false;

    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        request, //bstr_t("SELECT * FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL, &pEnumerator);
    
    if(!FAILED(hres))
    {
		IWbemClassObject *pclsObj;
		ULONG uReturn = 0;
	   
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
				&pclsObj, &uReturn);

			if(0 == uReturn)
			{
				break;
			}

			//VARIANT vtProp;
			//VariantInit(&vtProp);

			// Get the value of the Name property
			hr = pclsObj->Get(prop, 0, ret, 0, 0);
		    if(!FAILED(hres))
			{
				b=true;
				break;
			}

			//VariantClear(&vtProp);
		    pclsObj->Release();
		}
	    pEnumerator->Release();
    }
	return b;
}


bool getComputerInfo(char *ids)
{
    HRESULT hres;
	char	tmp[1024];
	tmp[0]=0;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres =  CoInitialize(0); 
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x" 
            << hex << hres << endl;
        return FALSE;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

/*                      
    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x" 
            << hex << hres << endl;
        CoUninitialize();
        return FALSE;                    // Program has failed.
    }
*/  
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return FALSE;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x" 
             << hex << hres << endl;
        pLoc->Release();     
        CoUninitialize();
        return FALSE;                // Program has failed.
    }

    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x" 
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return FALSE;               // Program has failed.
    }

	// do request
	{
		tmp[0]=0;

		{
			VARIANT vtProp;
			VariantInit(&vtProp);
			if(get(pSvc, bstr_t("select * from Win32_logicaldisk"), L"VolumeSerialNumber", &vtProp))
			{
				char	asc[1024];
				int		r=WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, asc, sizeof(asc), NULL, NULL);
				if(r)
					strcat(tmp, asc);

			}
			VariantClear(&vtProp);
		}
		strcat(tmp, ";");
		{
			VARIANT vtProp;
			VariantInit(&vtProp);
			if(get(pSvc, bstr_t("select * from Win32_NetworkAdapterConfiguration"), L"MacAddress", &vtProp))
			{
				char	asc[1024];
				int		r=WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, asc, sizeof(asc), NULL, NULL);
				if(r)
				{
					char	*s=asc;
					while(*s)
					{
						char	*sf=strchr(s, ':');
						if(sf)
						{
							*sf=0;
							strcat(tmp, s);
							s=sf+1;
						}
						else
						{
							strcat(tmp, s);
							break;
						}
					}
				}

			}
			VariantClear(&vtProp);
		}
		strcat(tmp, ";");
		{
			VARIANT vtProp;
			VariantInit(&vtProp);
			if(get(pSvc, bstr_t("select * from Win32_Processor"), L"ProcessorId", &vtProp))
			{
				char	asc[1024];
				int		r=WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, asc, sizeof(asc), NULL, NULL);
				if(r)
					strcat(tmp, asc);

			}
			VariantClear(&vtProp);
		}
	}
	 

    // Cleanup
    // ========
    
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

	{
		unsigned __int64	q=0;
		char				*s=tmp;
		while(*s)
		{
			unsigned __int64	v=(unsigned char)*s;
			q=(q<<1)^v;
			s++;
		}
		sprintf(ids, "%16I64X", q);
		s=ids;
		while(*s)
		{
			if(*s==' ')
				*s='0';
			s++;
		}
	}

    return TRUE;   // Program successfully completed.
	
}