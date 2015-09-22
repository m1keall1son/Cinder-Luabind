#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace luabind; //included before cinder in bindGlm_Prefix.pch

#include "CinderLuabind.h"
#include "CinderGlmBindings.h"
#include <luabind/operator.hpp>

//!create an interface for what a "script" object will be

class ScriptBase {
public:
    ScriptBase( const std::string &name ):mName(name){}
    virtual void setup(){};
    virtual void update(double){};
    std::string mName;
    virtual ~ScriptBase(){}
};

//! wrap the script interface with luabind to create a lua-side extensible base class for the script

LB_WRAP_BASE_BEGIN( ScriptBase );
LB_WRAP_BASE_CONSTRUCTOR_ARGS( ScriptBase, (const std::string &) name );
LB_WRAP_BASE_VIRTUAL_FN( ScriptBase, void, setup );
LB_WRAP_BASE_VIRTUAL_FN_ARGS( ScriptBase, void, update, (double) time );
LB_WRAP_BASE_END();

//! User defined objects to be manipulated in the script

using MovableObjectRef = std::shared_ptr< class MovableObject >;

class MovableObject {
public:
    MovableObject( const vec3 & startingPos ):mModelMatrix( ci::translate(startingPos) ) {}
    MovableObject( const luabind::object& lua_init ){
        
        mat4 translation = ci::translate( vec3(0) );
        mat4 rotation = mat4(1);
        mat4 scale = ci::scale( vec3(1) );
        
        for (luabind::iterator iter( lua_init ), end; iter != end; ++iter ) {
            
            std::string init_key = luabind::object_cast<std::string>( iter.key() );
            
            if( init_key == "translation" ){
                translation = ci::translate( luabind::object_cast<ci::vec3>( *iter ) );
            }else if( init_key == "rotation" ){
                rotation = luabind::object_cast<ci::mat4>( *iter );
            }else if( init_key == "scale" ){
                scale = ci::scale( luabind::object_cast<ci::vec3>( *iter ) );
            }
            
        }
		
        mModelMatrix = translation;
        mModelMatrix *= rotation;
        mModelMatrix *= scale;
    }
	
    mat4& getMatrix(){ return mModelMatrix; }
    void setMatrix( const ci::mat4 &mat ){ mModelMatrix = mat; }
    
private:
    
    mat4 mModelMatrix;
    friend class ObjectManager;
};

class MovableObjectManager {
public:
    
    void addObject( const MovableObject& obj ){ mObjects.push_back(obj);  }
    MovableObject& getObject( int index ){
        return mObjects[index];
    }
    size_t getNumObjects(){ return mObjects.size(); }
    void clear(){ mObjects.clear(); }
    
private:
    std::vector<MovableObject> mObjects;
};


class ScriptApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    
	ci::lb::ContextRef		mLuaContext;
    MovableObjectManager	mObjectManager;
	ScriptBase*				mMovableObjectTransformScript;
	
    CameraPersp mCamera;
    gl::BatchRef mTeapot;
    
};


void ScriptApp::setup()
{
	//! create a lua context
	
	mLuaContext = ci::lb::Context::create("main context");
	
	//! Bind classes with the luabind::module
	
	luabind::module(mLuaContext->getState())
	[
	 
	 //!Bind user-defined classes
	 
	 luabind::class_< ScriptBase, LB_WRAP_BASE_GET(ScriptBase) >("ScriptBase")
	 .def( constructor<const std::string&>() )
	 .def_readwrite("mName", &ScriptBase::mName)
	 .def( "update", &ScriptBase::update, &LB_WRAP_BASE_GET_DEFAULT_FN(ScriptBase, update) )
	 .def( "setup", &ScriptBase::setup, &LB_WRAP_BASE_GET_DEFAULT_FN(ScriptBase, setup) ),
		
	luabind::class_< MovableObject >("MovableObject")
	 .def( constructor<const ci::vec3&>() )
	 .def( constructor<const luabind::object&>() )
	 .def( "getMatrix", &MovableObject::getMatrix )
	 .def( "setMatrix", &MovableObject::setMatrix ),
	 
	 luabind::class_< MovableObjectManager >("MovableObjectManager")
	 .def( constructor<>() )
	 .def( "addObject", &MovableObjectManager::addObject )
	 .def( "getObject", &MovableObjectManager::getObject )
	 .def( "clear", &MovableObjectManager::clear ),
	 
	 //! bind cinder glm classes, namespace is purely organizaional
	 
	 luabind::namespace_("ci")[ LB_CI_GLM() ]
	 
	 ];
	
	//! run the lua script that extends hte base class and instantiates a global lua variable
	//! the ScriptBase variable is the only object held in our lua context, the MovableObjects created and manipulated
	//! by the script are actually held by the manager on the c++ side.
	
    try{
        mLuaContext->runLuaScript( loadAsset("MovableObjectTransformScript.lua") );
	}catch( const ci::lb::LuaException &e )
    {
        console() << e.what() << endl;
    }
	
	//! allow the c++ side MovableObjectManager to be accessed by the lua context
    mLuaContext->setGlobal( "mObjectManager", &mObjectManager );
	
	//! get the pointer to the extended and instantiated ScriptBase object from the lua side
    mMovableObjectTransformScript = mLuaContext->getGlobal<ScriptBase*>( "mMovableObjectTransformScript" );
	
	//! from c++ we can call any function from the base class interface defined above and wrapped with luabind
	//! in lua, you can define new functions on the class for use only on the lua side
	mMovableObjectTransformScript->setup();
	
    mCamera.setPerspective(60, getWindowAspectRatio(), .1, 10000.);
    mCamera.lookAt(vec3(0,0,3),vec3(0));
	
    mTeapot = gl::Batch::create( geom::Teapot(), gl::getStockShader(gl::ShaderDef().lambert() ) );
	
    gl::enableDepthRead();
    gl::enableDepthWrite();
	
}

void ScriptApp::mouseDown( MouseEvent event )
{
    try{
        mLuaContext->runLuaScript( loadAsset("MovableObjectTransformScript.lua") );
	}catch( const ci::lb::LuaException &e )
    {
        console() << e.what() << endl;
    }
	
	//! if we reload, the previous object is destroyed so we need to recapture the script pointer
	mMovableObjectTransformScript = mLuaContext->getGlobal<ScriptBase*>("mMovableObjectTransformScript");
    mMovableObjectTransformScript->setup();
	
}

void ScriptApp::update()
{
	mMovableObjectTransformScript->update( getElapsedSeconds() );
}

void ScriptApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    gl::ScopedMatrices pushMatrix;
    gl::setMatrices(mCamera);
	
    {
        for( int i=0;i<mObjectManager.getNumObjects();i++ ){
            gl::ScopedModelMatrix pushModel;
            gl::multModelMatrix( mObjectManager.getObject(i).getMatrix() );
            mTeapot->draw();
        }
    }
	
}

CINDER_APP( ScriptApp, RendererGl )
