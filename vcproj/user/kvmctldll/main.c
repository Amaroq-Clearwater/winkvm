
/* main.c */
#include <windows.h>
#include <tchar.h>

/* �O���[�o���ϐ� */
static HINSTANCE g_hInst;

/* ���C���֐� */
BOOL WINAPI 
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch( fdwReason ) {
	case DLL_PROCESS_ATTACH:  // ���[�h���ꂽ
		// �ǂ��Ŏg���̂ł���΁A�C���X�^���X�n���h�����O���[�o���ϐ��ɕۑ����Ă���
		g_hInst = hinstDLL;
		break;

	case DLL_PROCESS_DETACH:  // �A�����[�h���ꂽ
		// �ۑ����Ă������C���X�^���X�n���h�����폜
		g_hInst = NULL;
		break;

	case DLL_THREAD_ATTACH:   // �X���b�h���쐬����
		break;

	case DLL_THREAD_DETACH:   // �X���b�h���I������
		break;
	}
	return TRUE;
}

__declspec(dllexport) void __cdecl test(const char *str)
{
	MessageBoxA(NULL, str, "message", MB_OK);
}

