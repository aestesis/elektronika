#ifndef _ELEKTRO_REGISTRY_
#define _ELEKTRO_REGISTRY_
/*
// This function will register a component.
HRESULT RegisterServer(const char* szModuleName, REFCLSID clsid, 
    const char* szFriendlyName, const char* szVerIndProgID, 
    const char* szProgID, const char* szThreadingModel);

// This function will unregister a component.
HRESULT UnregisterServer(REFCLSID clsid, 
    const char* szVerIndProgID, const char* szProgID);

*/

HRESULT RegisterServer(HMODULE hModule,            // DLL module handle
                       const CLSID& clsid,         // Class ID
                       const char* szFriendlyName, // Friendly Name
                       const char* szVerIndProgID, // Programmatic
                       const char* szProgID) ;      //   IDs
                       
LONG UnregisterServer(const CLSID& clsid,         // Class ID
                      const char* szVerIndProgID, // Programmatic
                      const char* szProgID);       //   IDs




#endif