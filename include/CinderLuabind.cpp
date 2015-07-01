//
//  CinderLuabind.cpp
//  Script
//
//  Created by Mike Allison on 6/20/15.
//
//

#include "CinderLuabind.h"
#include "cinder/Utilities.h"

namespace lb {

StateRef State::create( Context* context ){
    return StateRef( new State(context) );
}

State::State( Context* context_name ):mContext(context_name){
    
    mState = luaL_newstate();
    if(!mState){
        throw LuaException( "Error creating state for context: " + mContext->getName() );
    }
    luaL_openlibs( mState );
    luabind::open( mState );
    lua_atpanic(mState, &State::panic);
}

int State::panic(lua_State *L){
    throw LuaException( std::string("Lua Panic: ") + lua_tostring( L, -1 ) );
    return 0;
}

State::~State(){
    lua_close(mState);
}

ContextRef Context::create( const std::string &name )
{
    return ContextRef( new Context( name ) );
}

Context::Context( const std::string &name ):mName(name),mState( State::create(this) ){}

void Context::addBindFunction( const BindFn &bindFn )
{
    mBound.push_back( std::make_pair(bindFn, false) );
}

void lb::Context::runLuaScript( const std::string &str )
{
  
    if(luaL_dostring(mState->getLuaState(), str.c_str()) != 0){
        throw LuaCompileException( lua_tostring(mState->getLuaState(), -1) );
    }
    
    ///collect garbage
    lua_gc(mState->getLuaState(), LUA_GCCOLLECT, 0);
    
}
    
void lb::Context::runLuaScript( const ci::DataSourceRef &file )
{
    runLuaScript( ci::loadString(file) );
}

void Context::bindAll()
{
    for( auto & p : mBound){
        if(!p.second){
            try {
                p.first( mState->getLuaState() );
            } catch (const luabind::error &e) {
                throw LuaException( e.what() );
            }
            p.second = true;
        }
    }
}

}