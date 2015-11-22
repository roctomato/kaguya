#pragma once

#include <string>

#include "kaguya/config.hpp"

#include "kaguya/utils.hpp"
#include "kaguya/metatable.hpp"
#include "kaguya/selector.hpp"
#include "kaguya/error_handler.hpp"


namespace kaguya
{
	class State
	{
		lua_State *state_;
		bool created_;

		ErrorHandler error_handler_;

		//non copyable
		State(const State&);
		State& operator =(const State&);

		static void stderror_out(int status, const char* message)
		{
			std::cerr << message << std::endl;
		}
		void init()
		{
			setErrorFunction(&stderror_out);
			nativefunction::reg_functor_destructor(state_);
		}
	public:
		State() :state_(luaL_newstate()), created_(true)
		{
			init();
		}
		State(lua_State* lua) :state_(lua), created_(false)
		{
			init();
		}
		~State()
		{
			if (created_)
			{
				lua_close(state_);
			}
		}

		void setErrorFunction(standard::function<void(int statuscode,const char*message)> errorfunction)
		{
			error_handler_.setFunction(errorfunction);			
		}

		void openlibs()
		{
			utils::ScopedSavedStack save(state_);
			luaL_openlibs(state_);
		}

		bool loadFile(const std::string& str)
		{
			return loadFile(str.c_str());
		}
		bool loadFile(const char* file)
		{
			utils::ScopedSavedStack save(state_);

			int status = luaL_loadfile(state_, file);

			if (status)
			{
				error_handler_.handle(status, state_);
				return false;
			}
			status = lua_pcall(state_, 0, LUA_MULTRET, 0);
			if (status)
			{
				error_handler_.handle(status, state_);
				return false;
			}
			return true;
		}

		bool dostring(const char* str)
		{
			utils::ScopedSavedStack save(state_);
			int status = luaL_dostring(state_, str);
			if (status)
			{
				error_handler_.handle(status, state_);
				return false;
			}
			return true;
		}
		bool dostring(const std::string& str)
		{
			return dostring(str.c_str());
		}

		bool operator()(const std::string& str)
		{
			return dostring(str);
		}
		bool operator()(const char* str)
		{
			return dostring(str);
		}
		Selector operator[](const std::string& str)
		{
			return Selector(state_, str,error_handler_);
		}

		Selector operator[](const char* str)
		{
			return Selector(state_, str,error_handler_);
		}

		void garbage_collect()
		{
			lua_gc(state_, LUA_GCCOLLECT, 0);
		}
		lua_State *state() { return state_; };
	};
};