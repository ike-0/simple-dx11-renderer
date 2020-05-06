// Loosely based on CppEvent
// https://www.codeproject.com/Articles/1256352/CppEvent-How-to-Implement-Events-using-Standard-Cp
// Haven't fully tested atomic events yet, but basic events works great and a convenient to use
// Fairly similar to C#


#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <future>


template <typename... Args>
class EventHandler
{

public:
	inline EventHandler() {};
	inline ~EventHandler() {};

	typedef std::function<void(Args...)> HandlerType;
	typedef UINT HandlerID;

	void operator()(Args... params) const
	{
		if (_handlerFunc)
		{
			_handlerFunc(params...);
		}
	}


	bool IsValid() const
	{
		return (_handlerFunc);
	}

	// copy constructor
	EventHandler(const EventHandler& src)
		: _handlerFunc(src._handlerFunc), _handlerId(src._handlerId)
	{
	}

	// move constructor
	EventHandler(EventHandler&& src)
		: _handlerFunc(std::move(src._handlerFunc)), _handlerId(src._handlerId)
	{
	}

	// copy assignment operator
	EventHandler& operator=(const EventHandler& src)
	{
		_handlerFunc = src._handlerFunc;
		_handlerId = src._handlerId;

		return *this;
	}

	// move assignment operator
	EventHandler& operator=(EventHandler&& src)
	{
		std::swap(_handlerFunc, src._handlerFunc);
		_handlerId = src._handlerId;

		return *this;
	}

	explicit EventHandler(const std::function<void(Args...)>& handlerFunc)
		: _handlerFunc(handlerFunc)
	{
		_handlerId = ++_handlerIdCounter;
	}

	bool operator==(const EventHandler& other) const
	{
		return _handlerId == other._handlerId;
	}

	UINT id() const
	{
		return _handlerId;
	}



private:
	std::function<void(Args...)> _handlerFunc;
	UINT _handlerId;
	static std::atomic_uint _handlerIdCounter;
};

template <typename... Args> std::atomic_uint EventHandler<Args...>::_handlerIdCounter(0);

template <typename... Args>
class Event
{

	mutable std::mutex _handlersLocker;

	bool RemoveID(const typename EventHandler<Args...>::HandlerID& handlerId)
	{
		std::lock_guard<std::mutex> lock(_handlersLocker);

		auto it = std::find_if(_handlers.begin(), _handlers.end(),
			[handlerId](const EventHandler<Args...>& handler) { return handler.id() == handlerId; });
		if (it != _handlers.end())
		{
			_handlers.erase(it);
			return true;
		}

		return false;
	}
public:

	inline Event() {};
	inline ~Event() {};

	void Invoke(Args... params) const
	{
		std::list<EventHandler<Args...>> handlersCopy = GetHandlersCopy();
		call_impl(handlersCopy, params...);
	}
	typename EventHandler<Args...>::HandlerID Add(const EventHandler<Args...>& handler)
	{
		std::lock_guard<std::mutex> lock(_handlersLocker);

		_handlers.push_back(handler);
		return handler.id();
	}
	inline typename EventHandler<Args...>::HandlerID Add
	(const typename EventHandler<Args...>::HandlerType& handler)
	{
		return Add(EventHandler<Args...>(handler));
	}
	bool Remove(const EventHandler<Args...>& handler)
	{
		std::lock_guard<std::mutex> lock(_handlersLocker);

		auto it = std::find(_handlers.begin(), _handlers.end(), handler);
		if (it != _handlers.end())
		{
			_handlers.erase(it);
			return true;
		}

		return false;
	}
	std::future<void> InvokeAsync(Args... params) const
	{
		return std::async(std::launch::async, [this](Args... asyncParams)
			{ call(asyncParams...); }, params...);
	}

	inline void operator()(Args... params) const
	{
		Invoke(params...);
	}

	inline typename EventHandler<Args...>::HandlerID operator+=(const EventHandler<Args...>& handler)
	{
		return Add(handler);
	}

	inline typename EventHandler<Args...>::HandlerID
		operator+=(const typename EventHandler<Args...>::HandlerType& handler)
	{
		return Add(handler);
	}

	inline bool operator-=(const EventHandler<Args...>& handler)
	{
		return Remove(handler);
	}

	// copy constructor
	Event(const Event& src)
	{
		std::lock_guard<std::mutex> lock(src._handlersLocker);

		_handlers = src._handlers;
	}

	// move constructor
	Event(Event&& src)
	{
		std::lock_guard<std::mutex> lock(src._handlersLocker);

		_handlers = std::move(src._handlers);
	}

	// copy assignment operator
	Event& operator=(const Event& src)
	{
		std::lock_guard<std::mutex> lock(_handlersLocker);
		std::lock_guard<std::mutex> lock2(src._handlersLocker);

		_handlers = src._handlers;

		return *this;
	}

	// move assignment operator
	Event& operator=(Event&& src)
	{
		std::lock_guard<std::mutex> lock(_handlersLocker);
		std::lock_guard<std::mutex> lock2(src._handlersLocker);

		std::swap(_handlers, src._handlers);

		return *this;
	}

private:
	std::list<EventHandler<Args...>> _handlers;

	std::list<EventHandler<Args...>> GetHandlersCopy() const
	{
		std::lock_guard<std::mutex> lock(_handlersLocker);

		// Since the function return value is by copy, 
		// before the function returns (and destruct the lock_guard object),
		// it creates a copy of the m_handlers container.
		return _handlers;
	}

	void call_impl(const std::list<EventHandler<Args...>>& handlers, Args... params) const
	{
		for (const auto& handler : handlers)
		{
			handler(params...);
		}
	}
};