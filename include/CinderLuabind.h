//
//  CinderLuabind.h
//  Script
//
//  Created by Mike Allison on 6/20/15.
//
//

#pragma once

#include "cinder/Log.h"

///helps luabind deal with shared pointers

template<class T>
T* get_pointer(std::shared_ptr<T>& p)
{
	return p.get();
}

namespace cinder {
	
namespace lb {

    class LuaException : public luabind::error {
    public:
		LuaException( const std::string & description ):luabind::error( nullptr ), mDescription(description){}
		LuaException( lua_State * state, const std::string & description = "" ):luabind::error( state ), mDescription(description){}
		
        virtual const char* what() const throw() {
			if( this->state() )
				return std::string( mDescription + lua_tostring(this->state(), -1) ).c_str();
			else
				return mDescription.c_str();
		}
		
        virtual ~LuaException() throw() {}
	
	private:
		std::string mDescription;
    };
	
    using StateRef = std::shared_ptr< class State >;
    
    class Context;
    
    class State {
        
    public:
        
        ~State();
        
    private:
        
        static StateRef create( Context* context );
        
        inline lua_State* getLuaState(){ return mState; }
        inline Context* getContext(){ return mContext; }
        
        State( Context* context );
        
        static int panic( lua_State *L );
        
        lua_State* mState;
        Context* mContext;
        
        friend Context;
    };
    
    using ContextRef = std::shared_ptr< Context >;
    
    class Context {
        
    public:
        
        static ContextRef create( const std::string &name );
        
        void runLuaScript( const ci::DataSourceRef &file );
        void runLuaScript( const std::string &str );
		
		void setLuaErrorHandler( int(*handler)(lua_State*) );
		
		template<typename T>
		luabind::object setGlobal( const std::string &name, T* pointer );
		
        template<typename T>
        T getGlobal( const std::string &lua_global );
        
        inline lua_State* getState(){ return mState->getLuaState(); }
        inline const std::string& getName(){ return mName; }
        inline const std::string& getName() const { return mName; }

        ~Context(){}
        
    private:
        
        Context( const std::string& name );
        
        std::string mName;
        StateRef mState;
		
    };
	
	template<typename T>
	luabind::object Context::setGlobal( const std::string &name, T* pointer )
	{
		return luabind::globals(mState->getLuaState())[name] = pointer;
	}
    
    template<typename T>
    T Context::getGlobal( const std::string &name ){
        T ret;
        try{
            ret = luabind::object_cast<T>( luabind::globals(mState->getLuaState())[name] );
        }catch( const luabind::cast_failed &e ){
            throw LuaException(e.what());
        }
        return ret;
    }
	
	//! FunctionHandler is a wrpper to hold a method on a lua class with a specific instance of that class
	//! can be used to std::bind an arbirary lua function
	struct FunctionHandler
	{
		FunctionHandler( luabind::object* self, const luabind::object& obj)
		: m_func(obj),_self(self)
		{
			if (luabind::type(obj) != LUA_TFUNCTION) {
				throw LuaException( "Invalid lua object, argument not a lua function" );
			}
		}
		
		void operator()()
		{
			try{
		  luabind::call_function<void>(m_func, *_self );
			}catch( const luabind::error &e )
			{
				throw LuaException( e.what() );
			}
		}
		
		
		template<typename... Args>
		void operator()( Args... args )
		{
			try{
				luabind::call_function<void>(m_func, *_self, args...);
			}catch( const luabind::error &e )
			{
				throw LuaException( e.what() );
			}
		}
		
	private:
		luabind::object m_func;
		luabind::object* _self;
	};
	
} //end namespace lb
} //end namespace cinder

//! convenience macros for wrapping lua extensible c++ classes
//! variadic macro voodoo learned from here: http://stackoverflow.com/questions/18237350/using-variadic-macros-or-templates-to-implement-a-set-of-functions

#define EAT(x)
#define REM(x) x
#define STRIP(x) EAT x
#define PAIR(x) REM x

#if defined( CINDER_MSW )

/* This counts the number of args */
#define NARGS_SEQ(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define NARGS_MSVC_WORKAROUND(x) NARGS_SEQ x
#define NARGS(...) NARGS_MSVC_WORKAROUND((__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1))

/* This will let macros expand before concating them */
#define PRIMITIVE_CAT(x, y) x ## y
#define CAT_MSVC_WORKAROUND(x) PRIMITIVE_CAT x
#define CAT(x, y) CAT_MSVC_WORKAROUND((x, y))

/* This will call a macro on each argument passed in */
#define APPLY(macro, ...) APPLY_MSVC_WORKAROUND(CAT(APPLY_, NARGS(__VA_ARGS__)), (macro, __VA_ARGS__))
#define APPLY_MSVC_WORKAROUND(m, x) m x

#else

/* This counts the number of args */
#define NARGS_SEQ(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define NARGS(...) NARGS_SEQ(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)

/* This will let macros expand before concating them */
#define PRIMITIVE_CAT(x, y) x ## y
#define CAT(x, y) PRIMITIVE_CAT(x, y)

/* This will call a macro on each argument passed in */
#define APPLY(macro, ...) CAT(APPLY_, NARGS(__VA_ARGS__))(macro, __VA_ARGS__)

#endif

#define APPLY_1(m, x1) m(x1)
#define APPLY_2(m, x1, x2) m(x1), m(x2)
#define APPLY_3(m, x1, x2, x3) m(x1), m(x2), m(x3)
#define APPLY_4(m, x1, x2, x3, x4) m(x1), m(x2), m(x3), m(x4)
#define APPLY_5(m, x1, x2, x3, x4, x5) m(x1), m(x2), m(x3), m(x4), m(x5)
#define APPLY_6(m, x1, x2, x3, x4, x5, x6) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6)
#define APPLY_7(m, x1, x2, x3, x4, x5, x6, x7) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7)
#define APPLY_8(m, x1, x2, x3, x4, x5, x6, x7, x8) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8)

#define LB_WRAP_BASE_GET( classname ) classname ## Wrapper
#define LB_WRAP_BASE_GET_DEFAULT_FN( classname, fn ) LB_WRAP_BASE_GET( classname )::default_##fn

#define LB_WRAP_BASE_BEGIN( classname ) \
	class classname ## Wrapper : public classname, public luabind::wrap_base { \
	public:

#define LB_WRAP_BASE_CONSTRUCTOR_ARGS( classname, ...) \
	LB_WRAP_BASE_GET( classname )( APPLY(PAIR, __VA_ARGS__) ):classname(  APPLY(STRIP, __VA_ARGS__) ){}

#define LB_WRAP_BASE_CONSTRUCTOR( classname ) \
	LB_WRAP_BASE_GET( classname )():classname(){}


#define LB_WRAP_BASE_VIRTUAL_FN_ARGS( classname, ret, fn_name, ... ) \
	virtual ret fn_name( APPLY(PAIR, __VA_ARGS__) ) override \
	{ \
		try{ \
			call<ret>(#fn_name, APPLY(STRIP, __VA_ARGS__)); \
		}catch( const luabind::error &e ) \
		{ \
			CI_LOG_E( e.what() ); \
		} \
	} \
	static void default_##fn_name(classname* ptr, APPLY(PAIR, __VA_ARGS__) ) \
	{ \
		return ptr->classname::fn_name(APPLY(STRIP, __VA_ARGS__)); \
	}

#define LB_WRAP_BASE_VIRTUAL_FN( classname, ret, fn_name ) \
	virtual ret fn_name() override \
	{ \
		try{ \
			call<ret>(#fn_name); \
		}catch( const luabind::error &e ) \
		{ \
			CI_LOG_E( e.what() ); \
		} \
	} \
	static void default_##fn_name(classname* ptr ) \
	{ \
		return ptr->classname::fn_name(); \
	}

#define LB_WRAP_BASE_END() };
