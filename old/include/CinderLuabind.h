//
//  CinderLuabind.h
//  Script
//
//  Created by Mike Allison on 6/20/15.
//
//

#pragma once

namespace lb {
    
    class LuaException : public ci::Exception {
    public:
        LuaException():ci::Exception(){}
        LuaException( const std::string &description ):ci::Exception( description ){}
        virtual ~LuaException() throw() {}
    };
    
    class LuaRuntimeException : public luabind::error {
    public:
        LuaRuntimeException( lua_State * state ):luabind::error( state ){}
        virtual const char* what() const throw() { return std::string( std::string("lua runtime error: ") + lua_tostring(state(), -1) ).c_str();  }
        virtual ~LuaRuntimeException() throw() {}
    };
    
    class LuaCompileException : public LuaException {
    public:
        LuaCompileException():LuaException(){}
        LuaCompileException( const std::string &description ):LuaException( description ){}
        virtual ~LuaCompileException() throw() {}
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
    
    using BindFn = std::function<void(lua_State*)>;
    using ContextRef = std::shared_ptr< Context >;
    
    class Context {
        
    public:
        
        static ContextRef create( const std::string &name );
        
        void addBindFunction( const BindFn &bind_function );
        void runLuaScript( const ci::DataSourceRef &file );
        void runLuaScript( const std::string &str );
        void bindAll();
        
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
        std::vector< std::pair<BindFn, bool> > mBound;
        
    };
    
    template<typename T>
    T Context::getGlobal( const std::string &lua_global ){
        T ret;
        try{
            ret = luabind::object_cast<T>( luabind::globals(mState->getLuaState())[lua_global] );
        }catch( const luabind::cast_failed &e ){
            throw LuaException(e.what());
        }
        return ret;
    }
    
}