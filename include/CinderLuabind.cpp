//
//  CinderLuabind.cpp
//  Script
//
//  Created by Mike Allison on 6/20/15.
//
//

#include "CinderLuabind.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

namespace cinder {
namespace lb {

StateRef State::create( Context* context ){
    return StateRef( new State(context) );
}

int defaultLuabindErrorHandler( lua_State* L )
{
	// log the error message
	luabind::object msg( luabind::from_stack( L, -1 ) );
	std::ostringstream str;
	str << "lua> run-time error: " << msg;
	
	// log the callstack
	std::string traceback = luabind::call_function<std::string>( luabind::globals(L)["debug"]["traceback"] );
	str << "\n" << std::string( "lua> " ) + traceback;
	CI_LOG_E( str.str() );
	
	// return unmodified error object
	return 1;
}
	
State::State( Context* context_name ):mContext(context_name){
    
    mState = luaL_newstate();
    if(!mState){
		throw LuaException( "Error creating state for context: " + mContext->getName() );
    }
    luaL_openlibs( mState );
    luabind::open( mState );
    lua_atpanic(mState, &State::panic);
	luabind::set_pcall_callback(&defaultLuabindErrorHandler);
}

int State::panic(lua_State *L){
    throw LuaException( L, "Lua Panic: " );
    return 0;
}

State::~State(){
    lua_close(mState);
}

ContextRef Context::create( const std::string &name )
{
    return ContextRef( new Context( name ) );
}

Context::Context( const std::string &name ):mName(name),mState( State::create(this) ){
    
    std::string packagePath = "package.path = \'";
    
    std::string stream;
    stream = "Cinder-Luabind adding lua 'require' paths \n";
    
    for( auto & path : ci::app::getAssetDirectories() ){
        stream += path.string() + "\n";
        packagePath += path.string()+"/?;";
        packagePath += path.string()+"/?.lua;";
    }
    
    ///need to add this one, how to get the name??
    //    std::string appResourcePath = ci::app::getAppPath().string() + "/Script.app/Contents/Resources/";
    //    auto p1 = appResourcePath + "?;";
    //    auto p2 = appResourcePath + "?.lua;";
    
    packagePath += "\' .. package.path";
    
    runLuaScript(packagePath);
    
    CI_LOG_V( stream );

}

void Context::setLuaErrorHandler( int(*handler)(lua_State*) )
{
	luabind::set_pcall_callback(handler);
}

void Context::runLuaScript( const std::string &str )
{
	
    if(luaL_dostring(mState->getLuaState(), str.c_str()) != 0){
        throw LuaException( lua_tostring(mState->getLuaState(), -1) );
    }
    
    ///collect garbage
    lua_gc(mState->getLuaState(), LUA_GCCOLLECT, 0);
    
}
    
void Context::runLuaScript( const ci::DataSourceRef &file )
{
    runLuaScript( ci::loadString(file) );
}

} //end namespace lb
} //end namespace cinder