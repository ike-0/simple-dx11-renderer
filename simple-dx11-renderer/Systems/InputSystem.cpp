#include "InputSystem.h"

InputSystem* InputSystem::Instance = nullptr;


InputSystem::InputSystem() :_lastKeyboardState{ 0 }, _lastBufferedMouseState{ 0 }, _lastMouseState{ 0 }
{
	Instance = this;
	_mouse = std::make_unique<DirectX::Mouse>();
	_keyboard = std::make_unique<DirectX::Keyboard>();
	Update();
}


InputSystem::~InputSystem()
{
	Instance = nullptr;
}

void InputSystem::Update()
{
	_lastKeyboardState = _keyboard->GetState();
	_lastBufferedMouseState = _lastMouseState;
	_lastMouseState = _mouse->GetState();
}

BOOL InputSystem::CheckKeyPress(DirectX::Keyboard::Keys key)
{
	return static_cast<BOOL>(_lastKeyboardState.IsKeyDown(key));
}

BOOL InputSystem::CheckMouseLeftPress()
{
	return static_cast<BOOL>(_lastMouseState.leftButton);
}

BOOL InputSystem::CheckMouseRightPress()
{
	return static_cast<BOOL>(_lastMouseState.rightButton);
}

DirectX::XMFLOAT2 InputSystem::GetDeltaMouse()
{
	return DirectX::XMFLOAT2{ 
		static_cast<float>(_lastMouseState.x) - static_cast<float>(_lastBufferedMouseState.x),
		static_cast<float>(_lastMouseState.y) - static_cast<float>(_lastBufferedMouseState.y),
	};
}
