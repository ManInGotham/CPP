/*
Usage:
	Client code needs to link against PackageDebug.lib.
	Include PackageDebug.h.
	Redefine (if needed for internal output printing) LogComment().
	Invoke functions EnableDebugging() or DisableDebugging().
*/

HRESULT EnableDebugging(_In_z_ LPCWSTR packageFullName, _In_z_ LPCWSTR debuggerString, _In_opt_z_ PZZTSTR environmentVars);
HRESULT DisableDebugging(_In_z_ LPCWSTR packageFullName);
void LogComment(_In_z_ LPCWSTR comment);