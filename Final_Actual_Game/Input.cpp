#include "Input.h"

Input::Input(HINSTANCE instance, HWND window)
{
	m_hInst = instance;
	m_hWnd = window;
}

Input::~Input() 
{
	if (m_keyboard_device)
	{
		m_keyboard_device->Unacquire();
		m_keyboard_device->Release();
	}

	if (m_direct_input) m_direct_input->Release();
}

// Initialise Input
HRESULT Input::InitialiseInput()
{
	HRESULT hr;
	ZeroMemory(m_keyboard_keys_state, sizeof(m_keyboard_keys_state));

	hr = DirectInput8Create(m_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_direct_input, NULL);
	if (FAILED(hr)) return hr;

	hr = m_direct_input->CreateDevice(GUID_SysKeyboard, &m_keyboard_device, NULL);
	if (FAILED(hr)) return hr;

	hr = m_keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) return hr;

	hr = m_keyboard_device->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) return hr;

	hr = m_keyboard_device->Acquire();
	if (FAILED(hr)) return hr;

	return S_OK;
}

// Update loop to get inputs
void Input::ReadInputStates()
{
	HRESULT hr;
	hr = m_keyboard_device->GetDeviceState(sizeof(m_keyboard_keys_state), (LPVOID)&m_keyboard_keys_state);

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST || (hr == DIERR_NOTACQUIRED)))
		{
			m_keyboard_device->Acquire();
		}
	}
}

// Check if a specific key is pressed
bool Input::IsKeyPressed(unsigned char DI_keycode)
{
	return m_keyboard_keys_state[DI_keycode] & 0x80;
}