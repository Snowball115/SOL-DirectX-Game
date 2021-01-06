#pragma once
#include <d3d11.h>
#include <dinput.h>

class Input
{
private:
	HINSTANCE m_hInst;
	HWND m_hWnd;
	IDirectInput8* m_direct_input;
	IDirectInputDevice8* m_keyboard_device;
	unsigned char m_keyboard_keys_state[256];

public:
	Input(HINSTANCE instance, HWND window);
	~Input();

	HRESULT InitialiseInput();
	void ReadInputStates();
	bool IsKeyPressed(unsigned char DI_keycode);
};

