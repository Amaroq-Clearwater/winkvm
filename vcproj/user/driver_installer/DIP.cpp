// DIP.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"

#define MAX_PATH_SIZE           (1024)
//#define ESM_POKECODEANDLOOKUP	(WM_USER + 101)

DWORD PopMsg(DWORD ret, LPCTSTR pszFormat, ...);
BOOL InstallDriver(const PTCHAR szDriverPath);
BOOL StartDriver(IN LPCTSTR szDriverName);
BOOL UnInstallDriver(IN LPCTSTR szDriver);
BOOL CreateDriver(IN LPCTSTR szDriverName,
				  IN LPCTSTR szDriverFullPath);

int main(int argc, char* argv[])
{
	int ch;
	BOOL ret = false;

	while ((ch = getopt(argc, argv, ":e:i:")) != -1) {
		switch (ch) {
			case 'i':
				fprintf(stdout, "install driver %s\n", optarg);
				ret = InstallDriver((PTCHAR)optarg);
				break;
			case 'e':
				fprintf(stdout, "uninstall driver %s\n", optarg);
				ret = UnInstallDriver((PTCHAR)optarg);
				break;
			case ':':
				fprintf(stdout, "%c needs filename\n", ch);
				break;
			case '?':				
				break;
			default:
				fprintf(stdout, "usage:\n");
				break;
		}
	}

	if (ret) {
		fprintf(stdout, "success!\n");
		exit(EXIT_SUCCESS);
	} 

	exit(EXIT_FAILURE);

	return 0;
}

BOOL InstallDriver(const PTCHAR szDriverPath)
{
	//�h���C�o���擾
	TCHAR *ptr = (TCHAR*)wcsrchr(szDriverPath, _T('\\'));
	if(ptr == NULL) {
		fprintf(stderr, "Illegal driver file path\");
		return false;
	}
	
	TCHAR szDriverName[MAX_PATH_SIZE];
	lstrcpy(szDriverName, ptr + 1);

	// �g���q���Ȃ�
	ptr = wcsrchr(szDriverName, _T('.'));
	if (ptr != NULL) {
		*ptr = _T('\0');
	}

	// �h���C�o�𐶐�
	if (CreateDriver(szDriverName, szDriverPath)) {
//		PopMsg(TRUE, _T("CreateDriver�֐������s���܂���"));
		return false;
	}

	// �h���C�o�T�[�r�X�J�n
	if(StartDriver(szDriverName)) {
		PopMsg(-1, _T("StartDriver�֐��͎��s���܂���"));
		return false;
	}

	return true;
}

BOOL CreateDriver(IN LPCTSTR szDriverName,
				  IN LPCTSTR szDriverFullPath)
{
	// �T�[�r�X�R���g���[���}�l�[�W���[���J��
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCManager == NULL) {
		return false;
	}

	// �h���C�o�T�[�r�X���J��
	SC_HANDLE hDriver = OpenService(hSCManager, szDriverName, SERVICE_ALL_ACCESS);

	// �����J���Ȃ��Ȃ�ΐV�����h���C�o�T�[�r�X�𐶐�
	if (hDriver == NULL) {
		hDriver = CreateService(hSCManager, szDriverName, szDriverName, SERVICE_ALL_ACCESS,
			                    SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
								szDriverFullPath, NULL, NULL, NULL, NULL, NULL);
		if (hDriver == NULL) {
			CloseServiceHandle(hSCManager);
			return true;
		}
		CloseServiceHandle(hDriver);
		CloseServiceHandle(hSCManager);
		return false;
	}
	
	// �����J����Ȃ�΃h���C�o�T�[�r�X���X�g�b�v
	BOOL bResult = false;

	try {
		SERVICE_STATUS ss;
		if(!ControlService(hDriver, SERVICE_CONTROL_INTERROGATE, &ss)) {
			return false;
		}

		if (ss.dwCurrentState == SERVICE_STOPPED) {
			return false;
		}
		
		if(!ControlService(hDriver, SERVICE_CONTROL_STOP, &ss)) {
			throw 1;
		}

		// �X�g�b�v������1�b���Ƃ�10��J��Ԃ�
		BOOL bStopped = FALSE;
		for (int i = 0; i < 10; i++) {
			Sleep(1000);
			if(ControlService(hDriver, SERVICE_CONTROL_INTERROGATE, &ss) == 0 || 
				ss.dwCurrentState == SERVICE_STOPPED) {
				bStopped = true;
				break;
			}
		}

		if(bStopped == false) {
			throw 2;
		}

	} catch(int err) {
		PopMsg(0, _T("Error: %d"), err);
		bResult = true;
	}

	CloseServiceHandle(hDriver);
	CloseServiceHandle(hSCManager);

	return bResult;
}

BOOL StartDriver(IN LPCTSTR szDriverName)
{
	// �T�[�r�X�R���g���[���}�l�[�W���[���J��
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCManager == NULL) {
		return TRUE;
	}

	// �T�[�r�X����}�l�[�W���̃f�[�^�x�[�X�n���h�����J��
	SC_HANDLE hDriver = OpenService(hSCManager, szDriverName, SERVICE_ALL_ACCESS);
	if (hDriver == NULL) {
		CloseServiceHandle(hSCManager);
		return TRUE;
	}

	BOOL bResult = FALSE;

	try{
		// �T�[�r�X������̌��݂̃X�e�[�^�X�����T�[�r�X�}�l�[�W���ɓ`���āA
		// �����X�V����悤�v������B
		SERVICE_STATUS ss;
		if(ControlService(hDriver, SERVICE_CONTROL_INTERROGATE, &ss)){
			if(ss.dwCurrentState == SERVICE_RUNNING)
				return FALSE;
		}
		else if(GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
			throw 2;

		// �T�[�r�X���J�n����
		if(!StartService(hDriver, 0, NULL))
			throw 3;

		// Give it 10 seconds to start
		BOOL bStarted = FALSE;
		for(int i = 0; i < 10; i++){
			Sleep(1000);
			if(ControlService(
				hDriver, SERVICE_CONTROL_INTERROGATE, &ss) &&
				ss.dwCurrentState == SERVICE_RUNNING)
			{
				bStarted = TRUE;
				break;
			}
		}
		if(!bStarted)
			throw 4;
	}catch(int err){
		PopMsg(0, _T("Error: %d"), err);
		bResult = TRUE;
	}

	CloseServiceHandle(hDriver);
	CloseServiceHandle(hSCManager);
	return bResult;
}

BOOL UnInstallDriver(IN LPCTSTR szDriver)
{
	// �h���C�o���擾
	TCHAR szDriverName[MAX_PATH_SIZE];
	lstrcpy(szDriverName, szDriver);
	TCHAR *ptr = (TCHAR*)wcsrchr(szDriver, _T('\\'));
	if (ptr != NULL) {
		lstrcpy(szDriverName, ptr + 1);
	}

	// �g���q���Ȃ�
	ptr = (TCHAR*)wcsrchr(szDriverName, _T('.'));
	if (ptr != NULL) {
		*ptr = _T('\0');
	}

	// �T�[�r�X�R���g���[���}�l�[�W���[���J��
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
	// �h���C�o�T�[�r�X���J��
	SC_HANDLE hDriver = OpenService(hSCManager, szDriverName, SERVICE_ALL_ACCESS);
	if (hDriver == NULL) {
		return TRUE;
	}

	// ��~�v��
	SERVICE_STATUS  ss;
	ControlService(hDriver, SERVICE_CONTROL_STOP, &ss);

	// �T�[�r�X����}�l�[�W���̃f�[�^�x�[�X����폜
	DeleteService(hDriver);

	// �I�u�W�F�N�g�n���h�������
    CloseServiceHandle(hDriver);
	CloseServiceHandle(hSCManager);
	return FALSE;
}

// printf�Ɏ��������̃��b�Z�[�W�{�b�N�X
DWORD PopMsg(DWORD ret, LPCTSTR pszFormat, ...)
{
	TCHAR    sz[1024];
	va_list  argList;

	va_start(argList, pszFormat);
	_vswprintf(sz, pszFormat, argList);
	va_end(argList);
	printf((char*)sz);

	return ret;
}