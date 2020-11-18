#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <aclapi.h>
#include <Lmwksta.h>
#include <StrSafe.h>

PSID pCurrentUserSID;

BOOL GetCurrentUserAndDomain(PTSTR szUser, PDWORD pcchUser,
	PTSTR szDomain, PDWORD pcchDomain)
{

	BOOL         fSuccess = FALSE;
	HANDLE       hToken = NULL;
	PTOKEN_USER  ptiUser = NULL;
	DWORD        cbti = 0;
	SID_NAME_USE snu;

	__try
	{

		// Get the calling thread's access token.
		if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE,
			&hToken))
		{

			if (GetLastError() != ERROR_NO_TOKEN)
				__leave;

			// Retry against process token if no thread token exists.
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY,
				&hToken))
				__leave;
		}

		// Obtain the size of the user information in the token.
		if (GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti))
		{

			// Call should have failed due to zero-length buffer.
			__leave;

		}
		else
		{

			// Call should have failed due to zero-length buffer.
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				__leave;
		}

		// Allocate buffer for user information in the token.
		ptiUser = (PTOKEN_USER)HeapAlloc(GetProcessHeap(), 0, cbti);
		if (!ptiUser)
			__leave;

		// Retrieve the user information from the token.
		if (!GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti))
			__leave;

		// Retrieve user name and domain name based on user's SID.
		if (!LookupAccountSid(NULL, ptiUser->User.Sid, szUser, pcchUser,
			szDomain, pcchDomain, &snu))
			__leave;

		pCurrentUserSID = ptiUser->User.Sid;

		fSuccess = TRUE;

	}
	__finally
	{

		// Free resources.
		if (hToken)
			CloseHandle(hToken);

		if (ptiUser)
			HeapFree(GetProcessHeap(), 0, ptiUser);
	}

	return fSuccess;
}

int main()
{
	PSID pEveryoneSID = NULL, pAdminSID = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
	EXPLICIT_ACCESS ea[2] = { 0 };

	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthLocal = SECURITY_LOCAL_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;

	TCHAR user[1024], domain[1024];
	DWORD chUser = sizeof(user), chDomain = sizeof(domain);
	if (!GetCurrentUserAndDomain(user, &chUser, domain, &chDomain))
	{
		printf("GetCurrentUserSID is failed\n");
	}

	try
	{
		// создаем SID для всех пользователей
		if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
			0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
		{
			throw std::exception("Can't create SID for current user!\n");
		}

		//// для администраторов
		//if (!AllocateAndInitializeSid(
		//	&SIDAuthNT,
		//	2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
		//	0, 0, 0, 0, 0, 0,
		//	&pAdminSID))
		//{
		//	throw std::exception("Can't create SID for admins!\n");
		//}

		// заполняем структуры EXPLICIT_ACCESS
		ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

		// 0 - для текущего
		ea[0].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE; // разрешение - ставим для чтения & выполненения
		ea[0].grfAccessMode = DENY_ACCESS; // для установки доступа
		ea[0].grfInheritance = NO_INHERITANCE; // без разрешения наследования 
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID; // форма структуры
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER; // группа, с которой мы будем работать
		ea[0].Trustee.ptstrName = (LPWSTR)pCurrentUserSID; // указатель на название

		// 1 - для всех
		ea[1].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE; // разрешение - ставим полное разрешение(чтение, запись)
		ea[1].grfAccessMode = SET_ACCESS; // для установки доступа
		ea[1].grfInheritance = NO_INHERITANCE; // без разрешения наследования 
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID; // форма структуры
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;// группа, с которой мы будем работать
		ea[1].Trustee.ptstrName = (LPWSTR)pEveryoneSID; // указатель на название

		// создаем Acess control list, содержащий выше проинициализированные записи 
		if (SetEntriesInAcl(
			2, // количество ea
			ea,
			NULL, // старый ACL 
			&pACL) // новый ACL 
			!= ERROR_SUCCESS)
		{
			throw std::exception("Can't create ACL!\n");
		}

		// Инициализируем SECURITY_DESCRIPTOR
		pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

		if (!pSecurityDescriptor)
		{
			throw std::exception("Can't create pointer to security descriptor!\n");
		}

		if (!InitializeSecurityDescriptor(pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
		{
			throw std::exception("can't create security descriptor!\n");
		}

		// Добавляем ACL к SECURITY_DESCRIPTOR
		if (!SetSecurityDescriptorDacl(
			pSecurityDescriptor, // указатель на SECURITY_DESCRPITOR
			TRUE, // Есть ли ACL
			pACL, // Сам ACL
			FALSE) // ACL по умолчанию
			)
		{
			throw std::exception("Can't add ACL to security descriptor!\n");
		}

		// создаем структуры SECURITY_ATTRIBUTES
		SECURITY_ATTRIBUTES securityAttributes = { 0 };
		securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		securityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
		securityAttributes.bInheritHandle = FALSE;

		// теперь создаем файлы с использованием атрибутов безопасности
		HANDLE hFile1 = CreateFile(L"dp_lab544.txt", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_DELETE,
			&securityAttributes, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);

		CloseHandle(hFile1);
	}
	catch (const std::exception& e)
	{
		printf("Error %s\n", e.what());
	}

	// очищаем все поля
	if (pEveryoneSID)
		FreeSid(pEveryoneSID);
	if (pAdminSID)
		FreeSid(pAdminSID);
	if (pACL)
		LocalFree(pACL);
	if (pSecurityDescriptor)
		LocalFree(pSecurityDescriptor);

	return 0;
}