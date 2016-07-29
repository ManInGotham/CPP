/*
Desc:
  This is a wrapper around public IPackageDebugSettings interface which allows to turn on\off debugging 
  of modern\metro applications on Windows OS, OneCore and Xbox OS.

  The reasoning to introduce the wrapper, involved the necessity to debug "restricted" applications which
  are not supported by the public interface right of way. There is a manual solution which includes 
  registry modification and Explorer process elevation. This code uses private APIs to do the work.

Classes:
  AppXPackage
  Main class which contains EnableDebugging() and DisableDebugging() methods.
  Supports reverting the changes to registry back to original state.
	
  TokenPrivelege
  Helper class to request additional privileges to current process.

  SIDUtils
  Helper class which automates some common actions with Windows security ID.
*/


#include <windows.h>
#include <shlobj.h>
#include <shprivUsesWinRT.h>
#include <shobjidl.h>
#include <wrl\client.h>
#include <WindowsStringP.h>
#include <string>
#include <sddl.h>
#include <aclapi.h>
#include <rodebugp.h>
#include <strsafe.h>

#include "PackageDebug.h"

using namespace std;
using namespace Microsoft::WRL;

// Should be defined by this lib's client
//
extern void LogComment(_In_z_ LPCWSTR comment);

void LogCommentFormatted(_In_z_ LPCWSTR formatText, ...)
{
    const short MAX_COMMENT = 1024;
    va_list args;
    va_start(args, formatText);
    WCHAR text[MAX_COMMENT];
    StringCchVPrintfW(text, MAX_COMMENT, formatText, args);
    LogComment(text);
    va_end(args);
}

void LogError(_In_ DWORD errorCode, _In_z_ LPCWSTR message, ...)
{
    const short MAX_COMMENT = 1024;

    LPWSTR errorCodeDescribed = NULL;

    ::FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR) &errorCodeDescribed,
        0, NULL);

    va_list args;
    va_start(args, message);
    WCHAR text[MAX_COMMENT];
    ::StringCchVPrintfW(text, ARRAYSIZE(text), message, args);
    va_end(args);

    LogCommentFormatted(L"Error: %sFailed due to code %x: %s", text, errorCode, errorCodeDescribed);
    ::HeapFree(::GetProcessHeap(), 0, errorCodeDescribed);
}


// Utility class for PSID type
//
class SIDUtils
{
public:
    SIDUtils() :
        m_accountName(nullptr),
        m_accountDomain(nullptr)
    {
    }

    HRESULT PrintUnderlyingAccountName(const PSID pSID)
    {
        DWORD dwError;
        LPCWSTR machineName = nullptr;
        DWORD accountNameLength = 0;
        DWORD domainNameLength = 0;
        SID_NAME_USE sidType;

        // Query the lengths of account and domain names by passing nullptrs
        //
        if (!::LookupAccountSidW(machineName, pSID, nullptr, &accountNameLength, nullptr, &domainNameLength, &sidType))
        {
            // Expected when querying for the size
            dwError = ::GetLastError();
            if (dwError != ERROR_INSUFFICIENT_BUFFER)
            {
                // Unexpected error
                Cleanup();
                return HRESULT_FROM_WIN32(dwError);
            }
        }

        // Allocate memory for account and domain names
        //
        m_accountName = static_cast<WCHAR*>(::HeapAlloc(::GetProcessHeap(), 0, accountNameLength * sizeof(m_accountName)));
        if(!m_accountName)
        {
            Cleanup();
            return E_OUTOFMEMORY;
        }

        m_accountDomain = static_cast<WCHAR*>(::HeapAlloc(::GetProcessHeap(), 0, domainNameLength * sizeof(m_accountDomain)));
        if(!m_accountDomain)
        {
            Cleanup();
            return E_OUTOFMEMORY;
        }

        // Get the account and domain name values
        //
        if (!::LookupAccountSidW(machineName, pSID, m_accountName, &accountNameLength, m_accountDomain, &domainNameLength, &sidType))
        {
            dwError = ::GetLastError();
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        // Get SID text representation
        //
        if(!::ConvertSidToStringSidW(pSID, &m_sidAsString))
        {
            dwError = ::GetLastError();
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        LogCommentFormatted(L"%s\\%s (%s)\n", m_accountDomain, m_accountName, m_sidAsString);

        Cleanup();
        return S_OK;
    }

private:
    WCHAR * m_accountName;
    WCHAR * m_accountDomain;
    LPWSTR m_sidAsString;

    void Cleanup()
    {
        if(m_sidAsString)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_sidAsString);
        }

        if(m_accountName)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_accountName);
        }

        if(m_accountDomain)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_accountDomain);
        }
    }
};

// Wrapper class around TOKEN_PRIVILEGES which helps with resources freeing.
//
class TokenPrivilege
{
public:
    TokenPrivilege(HANDLE token = nullptr, wstring privilegeName = L"") :
        m_token(token),
        m_privilegeName(privilegeName)
    {

    }

    const bool operator == (const TokenPrivilege& other) const
    {
        return m_token == other.m_token;
    }

    const bool operator != (const TokenPrivilege& other) const
    {
        return !(*this == other);
    }

    HRESULT EnablePrivilege()
    {
        return SetPrivilege(true);
    }

    HRESULT DisablePrivilege()
    {
        return SetPrivilege(false);
    }

private:
    HANDLE m_token;
    wstring m_privilegeName;
    TOKEN_PRIVILEGES m_tokenPrivileges;
    LUID m_tokenPrivilegeId;

    HRESULT SetPrivilege(bool isEnabled)
    {
        // Lookup privilege on local system
        //
        LPCWSTR machineName = nullptr;

        if (!::LookupPrivilegeValueW(machineName, m_privilegeName.c_str(), &m_tokenPrivilegeId))
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        m_tokenPrivileges.PrivilegeCount = 1;
        m_tokenPrivileges.Privileges[0].Luid = m_tokenPrivilegeId;
        m_tokenPrivileges.Privileges[0].Attributes = isEnabled ? SE_PRIVILEGE_ENABLED : 0;

        //
        // Enable the privilege.
        // If function succeeds, the return value is nonzero.
        // To determine whether the function adjusted all of the specified privileges we call GetLastError.
        // It returns one of the following values when the function succeeds:
        //
        // ERROR_SUCCESS --          The function adjusted all specified privileges.
        //
        // ERROR_NOT_ALL_ASSIGNED -- The token does not have one or more of the privileges specified in the NewState parameter.
        //                           The function may succeed with this error value even if no privileges were adjusted.
        //                           The PreviousState parameter indicates the privileges that were adjusted.
        //
        const BOOL ALL_PRIVILEGES_DISABLED = FALSE;
        const PTOKEN_PRIVILEGES PREVIOUS_TOKEN_STATE = nullptr;
        const PDWORD RETURN_LENGTH = nullptr;

        if (::AdjustTokenPrivileges(m_token, ALL_PRIVILEGES_DISABLED, &m_tokenPrivileges, sizeof(m_token), PREVIOUS_TOKEN_STATE, RETURN_LENGTH) == 0)
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        DWORD dwError = ::GetLastError();
        if(dwError == ERROR_NOT_ALL_ASSIGNED)
        {
            return HRESULT_FROM_WIN32(dwError);
        }

        return S_OK;
    }
};

// Allows client to turn on and off debugging settings via private COM APIs
//
class AppXPackage
{
private:
    LPCWSTR m_packageName;
    wstring m_packageRegPath;
    wstring m_packageFullRegPath;
    wstring m_packageDebugNodePath;
    HKEY m_pPackageRegNode;
    PSECURITY_DESCRIPTOR m_sdOldDaclAndOwner;
    SECURITY_DESCRIPTOR m_sdNewDaclAndOwner;
    PTOKEN_USER m_pProcessUserToken;
    HANDLE m_pProcessHandle;
    PSID m_pPackageRegOwner;
    BOOL m_isDaclPresented;
    PACL m_pOldDacl;
    BOOL m_isDaclDefaulted;
    PACL m_pNewDacl;
    TokenPrivilege m_takeOwnershipPrivilege;
    TokenPrivilege m_securityPrivilege;
    TokenPrivilege m_restorePrivilege;
    LPCWSTR m_debuggerString;
    PZZWSTR m_environmentVars;
    wstring m_debuggingModeString;
	ComPtr<IPackageDebugSettings> m_packageDebugSettings;
	HRESULT m_hrCoCreate;

public:
    AppXPackage(_In_z_ LPCWSTR packageName)
    {
        m_packageName = packageName;
        wstring temp(packageName);
        m_packageRegPath = L"ActivatableClasses\\Package\\" + temp;
        m_packageFullRegPath = L"CLASSES_ROOT\\" + m_packageRegPath;
        m_packageDebugNodePath = m_packageRegPath + L"\\DebugInformation";
        m_sdOldDaclAndOwner = nullptr;
        m_pPackageRegNode = nullptr;
        m_pProcessUserToken = nullptr;
        m_pNewDacl = nullptr;
		m_packageDebugSettings = nullptr;

		m_hrCoCreate = ::CoCreateInstance(CLSID_PackageDebugSettings, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_packageDebugSettings));
		if (FAILED(m_hrCoCreate))
		{
			LogError(m_hrCoCreate, L"Cannot create CLSID_PackageDebugSettings\n");
		}
    }

    HRESULT EnableDebugging(_In_z_ LPCWSTR debuggerString, _In_opt_z_ PZZWSTR environmentVars)
    {
       m_debuggerString = debuggerString;
       m_environmentVars = environmentVars;

       return SetDebugging(true);
    }

    HRESULT DisableDebugging()
    {
        return SetDebugging(false);
    }

private:
    /*
        Our algorithm is the following		
        0) Try to turn on\off debugging via public IPackageDebugSettings->*Debugging() and private Ro*DebugForPackage*() 
		1) If it fails due to access denied, proceed to the next steps.
        2) It assumes a client passed HKLM registered package. Modify registry permissions in the following way
        - Takes a package registry node under HKCR\ActivatableClasses. HKCR nodes are aligned with HKLM and HKCR
        - Assigns the current process user as an owner of the node. It needs elevated permissions.
        - Adds the current process user into ACL with Read/Write permissions. By default, unless there were external
        interactions with the DACL, only NT SERVICE\TrustedInstaller has Read/Write permissions.
        3) Invoke step 0 again        
        4) Restore registry permissions
    */
    HRESULT SetDebugging(bool isEnableDebugMode)
    {
		HRESULT hr = S_OK;

		if (m_packageDebugSettings == nullptr)
		{
			return m_hrCoCreate;
		}

        m_debuggingModeString = isEnableDebugMode ? L"Enable" : L"Disable";
        LogCommentFormatted(L"%s debug mode\n", m_debuggingModeString.c_str());

        // Handle the case when client wants to turn debugging off
        //
        if(!isEnableDebugMode)
        {
            // CallPlmToDisableDebugging uses code that runs as part of package deployment.
            // A requirement for this code is that failures are ignored by design for certain types of operations because
            // the deployment will handle that at a global scale (i.e. intermediate failures should not abort deployment as a whole).
            //
            // I.e. we can't trust failures when disabling debugging. This behavior might change in future.
            //
            HRESULT const disableHr = CallPlmToDisableDebugging();
            if (FAILED(disableHr))
            {
                LogError(disableHr, L"CallPlmToDisableDebugging reported an error.\n");
            }

            // Just tried to remove the debug node. Check if it was successful
            //
            HKEY debugRegKey;
            if(::RegOpenKeyExW(HKEY_CLASSES_ROOT, m_packageDebugNodePath.c_str(), 0, KEY_READ, &debugRegKey) == ERROR_SUCCESS)
            {
                LogCommentFormatted(L"Debug node %s still exists after first attempt \n", m_packageDebugNodePath.c_str());

                ::RegCloseKey(debugRegKey);

                return SetDebuggingElevated(isEnableDebugMode);
            }

            // Node was removed
            //
            return S_OK;
        }

        // Client wants to turn debugging on
        //
        hr = CallPlmToEnableDebugging();

        if(hr != S_OK)
        {
            LogError(hr, L"Failed to %s debug mode\n", m_debuggingModeString.c_str());
        }

        if(hr != E_ACCESSDENIED)
        {
            // Some unexpected error. No need to re-run elevated. Just return error to client
            //
            return hr;
        }

        // Re-run tweaking the registry permissions, which requires elevation
        //
        return SetDebuggingElevated(isEnableDebugMode);
    }

    HRESULT SetDebuggingElevated(bool isEnableDebugMode)
    {
        HRESULT hr;

        // There are problems with access permissions in registry
        LogCommentFormatted(L"Adjusting %s registry permissions before re-try\n", m_packageRegPath.c_str());

        hr = ChangeRegistryPermissions();
        if(hr != S_OK)
        {
            LogError(hr, L"SetRegistryPermissions()\n");
            return hr;
        }
        LogCommentFormatted(L"Done adjusting the registry\n");

        hr = isEnableDebugMode ?
            CallPlmToEnableDebugging() :
            CallPlmToDisableDebugging();

        if(hr != S_OK)
        {
            LogError(hr, L"%s debug mode failed again. Need further investigation\n", m_debuggingModeString.c_str());
        }
        else
        {
            LogCommentFormatted(L"Done %s debug mode\n", m_debuggingModeString.c_str());
        }

        HRESULT hrRevertRegistryPermissions = RevertRegistryPermissions();
        if(hrRevertRegistryPermissions != S_OK)
        {
            LogError(hrRevertRegistryPermissions, L"RevertRegistryPermissions()\n");
        }

        return hr;
    }

    HRESULT CallPlmToEnableDebugging()
    {
		// Calling public API. 
		// EnableDebugging() implementation lives in COMBASE.DLL loaded into Explorer.exe's address space. 
		// Explorer's security token is used on invocation. Since Explorer and this code might run under 
		// different users, the following call might fail. 
		HRESULT hr = m_packageDebugSettings->EnableDebugging(m_packageName, m_debuggerString, m_environmentVars);

		// Calling internal API on failure. 
		// It's almost identical code like in EnableDebugging() but it uses a security token of the curent process.
		if (FAILED(hr) && hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
		{
			if (m_debuggerString != nullptr || 
			    m_environmentVars != nullptr)
			{
				// Enable debug on activation
				hr = ::RoEnableDebuggingForPackage(m_packageName, m_debuggerString, m_environmentVars);
				if (FAILED(hr))
				{
					return hr;
				}
			}
			
			// Disable activation timeouts
			Microsoft::WRL::ComPtr<IApplicationActivationManagerPriv> spActivationManager;
			hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spActivationManager));
			if (FAILED(hr))
			{
				return hr;
			}
			if (SUCCEEDED(hr))
			{
				hr = spActivationManager->PutPackageActivationSettings(Windows::Internal::StringReference(m_packageName).Get(), PAS_NORPCTIMEOUTS, PAS_NORPCTIMEOUTS);

				// CImmersiveApplicationDebugControlInternal, which implements IPackageDebugSettings,
				// does the following. We don't have an ISuspendResumeController, so we don't do it.
				// This must be what disables suspend/resume on the app (to aid debugging).
				// We probably don't need it for iDNA.
				//if (SUCCEEDED(hr))
				//{
				//    hr = _spSuspendResumeController->OnDebugModeEnabled(m_packageName);
				//}
			}
			
			// Roll back on failure.
			if (FAILED(hr))
			{
				HRESULT const disableHr = CallPlmToDisableDebugging();
				if (FAILED(disableHr))
				{
					LogError(disableHr, L"CallPlmToDisableDebugging() reported an error while aborting in CallPlmToEnableDebugging() .\n");
				}
			}
		}
		
		return hr;		
    }

    HRESULT CallPlmToDisableDebugging()
    {
        HRESULT hr = m_packageDebugSettings->DisableDebugging(m_packageName);
        
		if (FAILED(hr) && hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
		{
			hr = ::RoDisableDebuggingForPackage(m_packageName);

			// It's possible that the package was never enabled for COM debugging (e.g. no debugger path string)
			// so ignore specific failures when disabling the package for debugging.
			if (FAILED(hr) && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
			{
				return hr;
			}

			// CImmersiveApplicationDebugControlInternal, which implements IPackageDebugSettings,
			// does the following. We don't have an ISuspendResumeController, so we don't do it.
			// This must be what re-enables suspend/resume on the app (disabled to aid debugging).
			// We probably don't need it for iDNA.
			// hr = _spSuspendResumeController->OnDebugModeDisabled(m_packageName);
			// if (FAILED(hr))
			// {
			//     hrError = hr;
			// }

			// Reenable activation timeouts
			Microsoft::WRL::ComPtr<IApplicationActivationManagerPriv> spActivationManager;
			hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spActivationManager));
			if (SUCCEEDED(hr))
			{
				hr = spActivationManager->PutPackageActivationSettings(Windows::Internal::StringReference(m_packageName).Get(), PAS_NORPCTIMEOUTS, PAS_DEFAULT);
			}
		}

		return hr;
    }

    /* This method
        - Takes a package registry node under HKCR\ActivatableClasses. HKCR nodes are aligned with HKLM and HKCR
        - Assigns the current process user as an owner of the node. It needs elevated permissions.
        - Adds the current process user into ACL with Read/Write permissions. By default, unless there were external
          interactions with the DACL, only NT SERVICE\TrustedInstaller has Read/Write permissions.
    */
    HRESULT ChangeRegistryPermissions()
    {
        DWORD dwError = S_OK;

        // Open a handle to the access token metadata, e.g. current user info. Also add a request to all to modify
        // process privileges.
        //
        if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &m_pProcessHandle))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"OpenProcessToken()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        DWORD requiredSizeForUserToken;
        if(::GetTokenInformation(m_pProcessHandle, TokenUser, static_cast<LPVOID>(m_pProcessUserToken), 0, &requiredSizeForUserToken) == 0)
        {
            dwError = ::GetLastError();
            if(dwError != ERROR_INSUFFICIENT_BUFFER)
            {
                Cleanup();
                return HRESULT_FROM_WIN32(dwError);
            }

            // Allocate required size, by default sizeof(TOKEN_USER)==8 on X64 e.g.
            //
            m_pProcessUserToken = reinterpret_cast<PTOKEN_USER>(new BYTE[requiredSizeForUserToken]);

            if(m_pProcessUserToken == nullptr)
            {
                // not enough memory
                //
                Cleanup();
                dwError = ERROR_INSUFFICIENT_BUFFER;
                LogError(dwError, L"new");
                return HRESULT_FROM_WIN32(dwError);
            }

            if(::GetTokenInformation(m_pProcessHandle, TokenUser, static_cast<LPVOID>(m_pProcessUserToken), requiredSizeForUserToken, &requiredSizeForUserToken) == 0)
            {
                dwError = ::GetLastError();
                Cleanup();
                LogError(dwError, L"GetTokenInformation(TokenUser)");
                return HRESULT_FROM_WIN32(dwError);
            }
        }

        LogCommentFormatted(L"Current process is running as ");
        SIDUtils sidUtils;
        sidUtils.PrintUnderlyingAccountName(m_pProcessUserToken->User.Sid);

        DWORD ignoredParameter;
        TOKEN_ELEVATION elevation;
        DWORD securityDescriptorSize;

        if (::GetTokenInformation(m_pProcessHandle, TokenElevation, &elevation, sizeof(elevation), &ignoredParameter) == 0)
        {
            // If the function succeeds, the return value is nonzero.
            //
            dwError = ::GetLastError();
            Cleanup();
            LogError(dwError, L"GetTokenInformation(TokenElevation)");
            return HRESULT_FROM_WIN32(dwError);
        }

        LogCommentFormatted(L"Is elevated process? %s\n", elevation.TokenIsElevated == 0 ? L"No" : L"Yes");

        if (!elevation.TokenIsElevated)
        {
            Cleanup();
            return HRESULT_FROM_WIN32(ERROR_ELEVATION_REQUIRED);
        }

        // Get current owner and DACL from the registry node
        //
        m_takeOwnershipPrivilege = TokenPrivilege(m_pProcessHandle, SE_TAKE_OWNERSHIP_NAME);
        m_takeOwnershipPrivilege.EnablePrivilege();

        m_securityPrivilege = TokenPrivilege(m_pProcessHandle, SE_SECURITY_NAME);
        m_securityPrivilege.EnablePrivilege();

        // Open registry to obtain DACL info
        //
        dwError = ::RegOpenKeyExW(
            HKEY_CLASSES_ROOT,
            m_packageRegPath.c_str(),
            0, // reserved, always 0
            READ_CONTROL |
            KEY_QUERY_VALUE | // to query security info
            ACCESS_SYSTEM_SECURITY, // to modify DACL
            &m_pPackageRegNode);

        if (dwError != ERROR_SUCCESS)
        {
            Cleanup();
            LogError(dwError, L"RegOpenKeyExW()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        // Determine security info size
        //
        dwError = ::RegQueryInfoKeyW(m_pPackageRegNode,
            nullptr,                 // buffer for class name
            nullptr,                 // length of class string
            nullptr,                 // reserved
            nullptr,                 // number of subkeys
            nullptr,                 // longest subkey size
            nullptr,                 // longest class string
            nullptr,                 // number of values for this key
            nullptr,                 // longest value name
            nullptr,                 // longest value data
            &securityDescriptorSize, // security descriptor
            nullptr);                // last write time

        if (dwError != ERROR_SUCCESS)
        {
            Cleanup();
            LogError(dwError, L"RegQueryInfoKeyW()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        // Need to one more copy to revert all the changes later
        //
        m_sdOldDaclAndOwner = static_cast<PSECURITY_DESCRIPTOR>(new BYTE[securityDescriptorSize]);
        if (m_sdOldDaclAndOwner == nullptr)
        {
            Cleanup();
            LogError(ERROR_NOT_ENOUGH_MEMORY , L"new");
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }

        dwError = ::RegGetKeySecurity(
            m_pPackageRegNode,
            DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
            m_sdOldDaclAndOwner,
            &securityDescriptorSize);

        if(dwError != ERROR_SUCCESS)
        {
            Cleanup();
            LogError(dwError, L"RegGetKeySecurity()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        BOOL isOwnerDefaulted;

        // Get current owner
        //
        if(!::GetSecurityDescriptorOwner(
            m_sdOldDaclAndOwner,
            &m_pPackageRegOwner,
            &isOwnerDefaulted))
        {
            dwError = ::GetLastError();
            Cleanup();
            LogError(dwError, L"GetSecurityDescriptorOwner()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        LogCommentFormatted(L"Current registry node owner is ");
        sidUtils.PrintUnderlyingAccountName(m_pPackageRegOwner);

        if (!::InitializeSecurityDescriptor(&m_sdNewDaclAndOwner, SECURITY_DESCRIPTOR_REVISION))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"InitializeSecurityDescriptor()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        if(!::SetSecurityDescriptorOwner(&m_sdNewDaclAndOwner, m_pProcessUserToken->User.Sid, FALSE))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"SetSecurityDescriptorOwner()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        ::RegCloseKey(m_pPackageRegNode);

        dwError = ::RegOpenKeyExW(
            HKEY_CLASSES_ROOT,
            m_packageRegPath.c_str(),
            0, // reserved, always 0
            WRITE_OWNER,
            &m_pPackageRegNode);

        if (dwError != ERROR_SUCCESS)
        {
            LogError(dwError, L"RegOpenKeyExW()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        dwError = ::RegSetKeySecurity(
            m_pPackageRegNode,
            OWNER_SECURITY_INFORMATION,
            &m_sdNewDaclAndOwner);

        if (dwError != ERROR_SUCCESS)
        {
            LogError(dwError, L"RegSetKeySecurity()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        ::RegCloseKey(m_pPackageRegNode);

        LogCommentFormatted(L"Set new registry node owner to ");
        sidUtils.PrintUnderlyingAccountName(m_pProcessUserToken->User.Sid);

        if(!::GetSecurityDescriptorDacl(
            m_sdOldDaclAndOwner,
            &m_isDaclPresented,
            &m_pOldDacl,
            &m_isDaclDefaulted))
        {
            dwError = ::GetLastError();
            Cleanup();
            LogError(dwError, L"GetSecurityDescriptorDacl()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        ACL_SIZE_INFORMATION aclInfo;
        DWORD cbNewAcl = 0;

        if (!::GetAclInformation(m_pOldDacl, &aclInfo, sizeof(aclInfo), AclSizeInformation))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"GetAclInformation()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        // Compute size needed for the new ACL
        cbNewAcl = aclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) + ::GetLengthSid(m_pProcessUserToken->User.Sid) - sizeof(DWORD);

        // Allocate and initialize the new ACL.
        m_pNewDacl = reinterpret_cast<PACL>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, cbNewAcl));
        if (m_pNewDacl == nullptr)
        {
            dwError = ::GetLastError();
            LogError(dwError, L"HeapAlloc()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        if (!::InitializeAcl(m_pNewDacl, cbNewAcl, ACL_REVISION))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"InitializeAcl()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        LPVOID pTempAce = nullptr;
        UINT currentAceIndex;

        // Add new Ace
        if (!::AddAccessAllowedAceEx(m_pNewDacl, ACL_REVISION, CONTAINER_INHERIT_ACE, GENERIC_ALL, m_pProcessUserToken->User.Sid))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"InitializeAcl()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        // Copy old Acl ACEs, except the one has same SID as the new Ace
        for (currentAceIndex = 0; currentAceIndex < aclInfo.AceCount; currentAceIndex++)
        {
            if (!::GetAce(m_pOldDacl, currentAceIndex, &pTempAce))
            {
                dwError = ::GetLastError();
                LogError(dwError, L"GetAce()\n");
                Cleanup();
                return HRESULT_FROM_WIN32(dwError);
            }

            // skip the SID for new Ace. We only need the updated one
            if (::EqualSid(&((static_cast<ACCESS_ALLOWED_ACE *> (pTempAce))->SidStart), m_pProcessUserToken->User.Sid))
            {
                continue;
            }

            if (!::AddAce(m_pNewDacl, ACL_REVISION, MAXDWORD, pTempAce, (static_cast<PACE_HEADER> (pTempAce))->AceSize))
            {
                dwError = ::GetLastError();
                LogError(dwError, L"GetAce()\n");
                Cleanup();
                return HRESULT_FROM_WIN32(dwError);
            }
        }

        if (!::SetSecurityDescriptorDacl(&m_sdNewDaclAndOwner, TRUE, m_pNewDacl, FALSE))
        {
            dwError = ::GetLastError();
            LogError(dwError, L"SetSecurityDescriptorDacl()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        dwError = ::RegOpenKeyExW(
            HKEY_CLASSES_ROOT,
            m_packageRegPath.c_str(),
            0, // reserved, always 0
            WRITE_DAC,
            &m_pPackageRegNode);

		if(dwError != ERROR_SUCCESS)
		{
			LogError(dwError, L"RegOpenKeyExW()\n");
			Cleanup();
			return HRESULT_FROM_WIN32(dwError);
		}

        dwError = ::RegSetKeySecurity(
            m_pPackageRegNode,
            DACL_SECURITY_INFORMATION,
            &m_sdNewDaclAndOwner);

        if (dwError != ERROR_SUCCESS)
        {
            LogError(dwError, L"RegSetKeySecurity()\n");
            Cleanup();
            return HRESULT_FROM_WIN32(dwError);
        }

        ::RegCloseKey(m_pPackageRegNode);

        LogCommentFormatted(L"Added current user into DACL with R/W permissions\n");

        return dwError;
    }

    // Change the registry permissions back to prevent the security hole
    //
    HRESULT RevertRegistryPermissions()
    {
        DWORD dwError;
        HRESULT hr;

        LogCommentFormatted(L"Restoring registry settings \n");

        if(!::GetSecurityDescriptorDacl(
            m_sdOldDaclAndOwner,
            &m_isDaclPresented,
            &m_pOldDacl,
            &m_isDaclDefaulted))
        {
            dwError = ::GetLastError();
            Cleanup();
            LogError(dwError, L"GetSecurityDescriptorDacl()\n");
            return HRESULT_FROM_WIN32(dwError);
        }

        // To be able to reassign the node ownership back for any account, we need to enable Restore privilege, e.g.
        // NT SERVICE\TrustedInstaller.
        //
        m_restorePrivilege = TokenPrivilege(m_pProcessHandle, SE_RESTORE_NAME);
        hr = m_restorePrivilege.EnablePrivilege();
        if (FAILED(hr))
        {
            LogError(hr, L"EnablePrivilege()\n");
            Cleanup();
            return hr;
        }

		dwError = ::RegOpenKeyExW(
			HKEY_CLASSES_ROOT,
			m_packageRegPath.c_str(),
			0, // reserved, always 0
			WRITE_DAC,
			&m_pPackageRegNode);

		if(dwError != ERROR_SUCCESS)
		{
			LogError(dwError, L"RegOpenKeyExW()\n");
			Cleanup();
			return HRESULT_FROM_WIN32(dwError);
		}

		dwError = ::RegSetKeySecurity(
			m_pPackageRegNode,
			DACL_SECURITY_INFORMATION,
			m_sdOldDaclAndOwner);

		if (dwError != ERROR_SUCCESS)
		{
			LogError(dwError, L"RegSetKeySecurity()\n");
			Cleanup();
			return HRESULT_FROM_WIN32(dwError);
		}

		::RegCloseKey(m_pPackageRegNode);

		dwError = ::RegOpenKeyExW(
			HKEY_CLASSES_ROOT,
			m_packageRegPath.c_str(),
			0, // reserved, always 0
			WRITE_OWNER,
			&m_pPackageRegNode);

		if(dwError != ERROR_SUCCESS)
		{
			LogError(dwError, L"RegOpenKeyExW()\n");
			Cleanup();
			return HRESULT_FROM_WIN32(dwError);
		}

		dwError = ::RegSetKeySecurity(
			m_pPackageRegNode,
			OWNER_SECURITY_INFORMATION,
			m_sdOldDaclAndOwner);

		if (dwError != ERROR_SUCCESS)
		{
			LogError(dwError, L"RegSetKeySecurity()\n");
			Cleanup();
			return HRESULT_FROM_WIN32(dwError);
		}

		::RegCloseKey(m_pPackageRegNode);

        LogCommentFormatted(L"Restored old DACL\n");
        LogCommentFormatted(L"Restored old owner back to ");
        SIDUtils sidUtils;
        sidUtils.PrintUnderlyingAccountName(m_pPackageRegOwner);

        return S_OK;
    }

    void Cleanup()
    {
        if(m_restorePrivilege != NULL)
        {
            m_restorePrivilege.DisablePrivilege();
        }

        if(m_takeOwnershipPrivilege != nullptr)
        {
            m_takeOwnershipPrivilege.DisablePrivilege();
        }

        if(m_securityPrivilege != nullptr)
        {
            m_securityPrivilege.DisablePrivilege();
        }

        if(m_pNewDacl != nullptr)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_pNewDacl);
        }

        ::CloseHandle(m_pProcessHandle);
        ::RegCloseKey(m_pPackageRegNode);

        if(m_sdOldDaclAndOwner != nullptr)
        {
            delete[] m_sdOldDaclAndOwner;
            m_sdOldDaclAndOwner = nullptr;
        }

        if(m_pProcessUserToken != nullptr)
        {
            delete[] m_pProcessUserToken;
        }
    }
};

// LIB main exported functions
//
HRESULT DisableDebugging(_In_z_ LPCWSTR packageFullName)
{
    AppXPackage package(packageFullName);
    return package.DisableDebugging();
}

HRESULT EnableDebugging(_In_z_ LPCWSTR packageFullName, _In_z_ LPCWSTR debuggerString, _In_opt_z_ PZZWSTR environmentVars)
{
    AppXPackage package(packageFullName);
    return package.EnableDebugging(debuggerString, environmentVars);
}