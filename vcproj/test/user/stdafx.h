// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#include "targetver.h"

#define GPD_DEVICE_NAME   L"\\Device\\winkvm"
#define NT_DEVICE_NAME    GPD_DEVICE_NAME

#define DOS_DEVICE_NAME   L"\\DosDevices\\winkvm"
#define PORTIO_TAG        'TROP'

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winioctl.h>
#include <memory.h>

#include "../include/kvm.h"

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă��������B
