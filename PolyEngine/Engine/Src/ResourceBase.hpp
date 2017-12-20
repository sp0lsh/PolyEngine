#pragma once

#include <exception>

#include <Defines.hpp>
#include <String.hpp>
#include <RefCountedBase.hpp>

namespace Poly
{
	enum class eResourceState
	{
		VALID,
		INVALID,
		_COUNT
	};

	//------------------------------------------------------------------------------
	class ENGINE_DLLEXPORT ResourceLoadFailedException : public BaseObject<>, public std::exception
	{
	public:
		ResourceLoadFailedException() {}
	};

	//------------------------------------------------------------------------------
	class ENGINE_DLLEXPORT ResourceBase : public RefCountedBase
	{
	public:
		const String& GetPath() const { return Path; }
		const eResourceState GetState() const { return State; }

		ResourceBase() = default;
		ResourceBase(const ResourceBase&) = delete;
		ResourceBase& operator=(const ResourceBase&) = delete;

	protected:
		virtual ~ResourceBase() {}

		eResourceState State;

	private:
		String Path;

		template<typename T> friend class ResourceManager;
	};
}