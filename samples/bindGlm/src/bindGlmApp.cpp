#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace luabind; //included before cinder

#include "CinderLuabind.h"
#include <luabind/operator.hpp>

using ScriptBaseRef = std::shared_ptr< class ScriptBase >;

class ScriptBase {
public:
    ScriptBase( const std::string &name ):mName(name){}
    virtual void setup(){};
    virtual void update(){};
    std::string mName;
    virtual ~ScriptBase(){}
};

class ScriptBaseWrapper : public ScriptBase, public wrap_base {
    
public:
    
    ScriptBaseWrapper(const std::string &name):ScriptBase(name){}
    
    virtual void update() override
    {
        try{
            call<void>("update");
        }catch( const error &e )
        {
            console() << e.what() << " : " << lua_tostring(e.state(), -1) << endl;
        }
    }
    
    virtual void setup() override
    {
        try{
            call<void>("setup");
        }catch( const error &e )
        {
            console() << e.what() << " : " << lua_tostring(e.state(), -1) << endl;
        }
    }
    
    static void default_update(ScriptBase* ptr)
    {
        return ptr->ScriptBase::update();
    }
    
    static void default_setup(ScriptBase* ptr)
    {
        return ptr->ScriptBase::setup();
    }
    
};

class Object {
public:
    Object( const vec3 & startingPos ):mModelMatrix( ci::translate(startingPos) ) {}
    Object( const luabind::object& lua_init ){
        
        mat4 translation = ci::translate( vec3(0) );
        mat4 rotation = mat4(1);
        mat4 scale = ci::scale( vec3(1) );
        
//        for (luabind::iterator i( lua_init ), end; i != end; ++i ) {
//            
//            std::string init_key = luabind::object_cast<std::string>( i.key() );
//            
//            if( init_key == "translation" ){
//                translation = ci::translate( luabind::object_cast<ci::vec3>(*i) );
//            }else if( init_key == "rotation" ){
//                rotation = luabind::object_cast<ci::mat4>( *i );
//            }else if( init_key == "scale" ){
//                scale = ci::scale( luabind::object_cast<ci::vec3>(*i) );
//            }
//            
//        }

        translation = ci::translate(luabind::object_cast<ci::vec3>( lua_init["translation"] ));
        
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

class ObjectManager {
public:
    
    void addObject( const Object& obj ){ mObjects.push_back(obj);  }
    Object& getObject( int index ){
        return mObjects[index];
    }
    size_t getNumObjects(){ return mObjects.size(); }
    void clear(){ mObjects.clear(); }
    
private:
    std::vector<Object> mObjects;
};

class ScriptApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    
    lb::ContextRef mLuaContext;
    ObjectManager mObjectManager;
    
    CameraPersp mCamera;
    gl::BatchRef mTeapot;
    
};


void ScriptApp::setup()
{
    
    mLuaContext = lb::Context::create("main context");
    
    std::function<void(lua_State*)> scriptBaseBind = [&]( lua_State* state ){
        module(state)
        [
         class_< ScriptBase, ScriptBaseWrapper >("Scriptable")
         .def( constructor<const std::string&>() )
         .def_readwrite("mName", &ScriptBase::mName)
         .def( "update", &ScriptBase::update, &ScriptBaseWrapper::default_update )
         .def( "setup", &ScriptBase::setup, &ScriptBaseWrapper::default_setup )
         ];
    };
    
    std::function<void(lua_State*)> objectBind = [&]( lua_State* state ){
        module(state)
        [
         class_< Object >("Object")
         .def( constructor<const ci::vec3&>() )
         .def( constructor<const luabind::object&>() )
         .def( "getMatrix", &Object::getMatrix )
         .def( "setMatrix", &Object::setMatrix ),
         class_< ObjectManager >("ObjectManager")
         .def( constructor<>() )
         .def( "addObject", &ObjectManager::addObject )
         .def( "getObject", &ObjectManager::getObject )
         .def( "clear", &ObjectManager::clear )
         ];
    };
    
    std::function<void(lua_State*)> glmBind = [&](lua_State* state){
       
        module(state)
        [
         
         namespace_("Math")[
            def("sin", (float(*)(float))&sin),
            def("cos", (float(*)(float))&cos)
         ],
         
         namespace_("ci")[
                                  
                          class_<ci::vec2>("vec2")
                          .def( constructor<float>() )
                          .def( constructor<float, float>() )
                          .def( constructor<const ci::vec2 &>() )
                          .def_readwrite("x", &ci::vec2::x)
                          .def_readwrite("y", &ci::vec2::y)
                          .def( self + other<const ci::vec2 &>() )
                          .def( self - other<const ci::vec2 &>() )
                          .def( self / other<const ci::vec2 &>() )
                          .def( self * other<const ci::vec2 &>() )
                          .def( self == other<const ci::vec2 &>() )
                          .def( tostring(self) ),
                          class_<ci::vec3>("vec3")
                          .def( constructor<float, float, float>() )
                          .def( constructor<const ci::vec3 &>() )
                          .def( constructor<float>() )
                          .def_readwrite("x", &ci::vec3::x)
                          .def_readwrite("y", &ci::vec3::y)
                          .def_readwrite("z", &ci::vec3::z)
                          .def( self + other<const ci::vec3 &>() )
                          .def( self - other<const ci::vec3 &>() )
                          .def( self / other<const ci::vec3 &>() )
                          .def( self * other<const ci::vec3 &>() )
                          .def( self * other<const ci::mat3 &>() )
                          .def( self == other<const ci::vec3 &>() )
                          .def( tostring(self) ),
                          class_<ci::vec4>("vec4")
                          .def( constructor<float, float, float, float>() )
                          .def( constructor<const ci::vec4 &>() )
                          .def( constructor<float>() )
                          .def_readwrite("x", &ci::vec4::x)
                          .def_readwrite("y", &ci::vec4::y)
                          .def_readwrite("z", &ci::vec4::z)
                          .def_readwrite("w", &ci::vec4::w)
                          .def( self + other<const ci::vec4 &>() )
                          .def( self - other<const ci::vec4 &>() )
                          .def( self / other<const ci::vec4 &>() )
                          .def( self * other<const ci::vec4 &>() )
                          .def( self * other<const ci::mat4 &>() )
                          .def( self * other<const ci::quat &>() )
                          .def( self == other<const ci::vec4 &>() )
                          .def( tostring(self) ),
                          class_<ci::quat>("quat")
                          .def( constructor<float, float, float, float>() )
                          .def( constructor<const ci::quat &>() )
                          .def_readwrite("x", &ci::quat::x)
                          .def_readwrite("y", &ci::quat::y)
                          .def_readwrite("z", &ci::quat::z)
                          .def_readwrite("w", &ci::quat::w)
                          .def( self + other<const ci::quat &>() )
                          .def( tostring(self) ),
                          class_<ci::mat4>("mat4")
                          .def( constructor<const ci::vec4&, const ci::vec4&, const ci::vec4&, const ci::vec4&>() )
                          .def( constructor<>() )
                          .def( constructor<float>() )
                          .def( constructor<const ci::mat4&>() )
                          .def( self + other<const ci::mat4 &>() )
                          .def( self - other<const ci::mat4 &>() )
                          .def( self / other<const ci::mat4 &>() )
                          .def( self * other<const ci::mat4 &>() )
                          .def( tostring(self) ),
                          class_<ci::mat3>("mat3")
                          .def( constructor<const ci::vec3&, const ci::vec3&, const ci::vec3&>() )
                          .def( constructor<>() )
                          .def( constructor<const ci::mat3&>() )
                          .def( self + other<const ci::mat3 &>() )
                          .def( self - other<const ci::mat3 &>() )
                          .def( self / other<const ci::mat3 &>() )
                          .def( self * other<const ci::mat3 &>() )
                          .def( tostring(self) ),
                          def( "length", (float(*)(const ci::vec3 &)) &ci::length ),
                          def( "length", (float(*)(const ci::quat &)) &ci::length ),
                          def( "translate", ( ci::mat4(*)( const ci::mat4 &, const ci::vec3 & ) )&ci::translate ),
                          def( "translate", ( ci::mat4(*)( const ci::vec3 & ) )&ci::translate ),
                          def( "scale", ( ci::mat4(*)( const ci::mat4 &, const ci::vec3 & ) )&ci::scale ),
                          def( "scale", ( ci::mat4(*)( const ci::vec3 & ) )&ci::scale ),
                          def( "rotate", ( ci::mat4(*)( float, const ci::vec3 & ) )&ci::rotate ),
                          def( "rotate", ( ci::mat4(*)( const ci::mat4&, float, const ci::vec3 & ) )&ci::rotate ),
                          def( "rotate", ( ci::vec3(*)( const ci::quat&, const ci::vec3 & ) )&ci::rotate ),
                          def( "rotate", ( ci::vec4(*)( const ci::quat&, const ci::vec4 & ) )&ci::rotate )
                          ]
         ];
    };
    
    mLuaContext->addBindFunction(glmBind);
    mLuaContext->addBindFunction(scriptBaseBind);
    mLuaContext->addBindFunction(objectBind);
    mLuaContext->bindAll();
    
    try{
        mLuaContext->runLuaScript( loadAsset("test.lua") );
    }catch( const lb::LuaCompileException &e )
    {
        console() << e.what() << endl;
    }
    
    globals(mLuaContext->getState())["manager"] = &mObjectManager;
    
    auto script1 = mLuaContext->getGlobal<ScriptBase*>("s");
    
    script1->setup();
    
    mCamera.setPerspective(60, getWindowAspectRatio(), .1, 10000.);
    mCamera.lookAt(vec3(0,0,3),vec3(0));
    
    mTeapot = gl::Batch::create( geom::Teapot(), gl::getStockShader(gl::ShaderDef().lambert() ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
}

void ScriptApp::mouseDown( MouseEvent event )
{
    try{
        mLuaContext->runLuaScript( loadAsset("test.lua") );
    }catch( const lb::LuaCompileException &e )
    {
        console() << e.what() << endl;
    }
    
    auto script1 = mLuaContext->getGlobal<ScriptBase*>("s");
    script1->setup();
}

void ScriptApp::update()
{
    auto script1 = mLuaContext->getGlobal<ScriptBase*>("s");
    script1->update();
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
