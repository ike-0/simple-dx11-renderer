#pragma once

class InputSystem
{
public:
	static InputSystem* Instance;
public:

	InputSystem();
	~InputSystem();

	void Update();
	BOOL CheckKeyPress(DirectX::Keyboard::Keys key);
	BOOL CheckMouseLeftPress();
	BOOL CheckMouseRightPress();

	DirectX::XMFLOAT2 GetDeltaMouse();

private:
	std::unique_ptr<DirectX::Keyboard>	_keyboard;
	std::unique_ptr<DirectX::Mouse>		_mouse;

	DirectX::Keyboard::State			_lastKeyboardState;
	DirectX::Mouse::State				_lastMouseState;
	DirectX::Mouse::State				_lastBufferedMouseState;
};