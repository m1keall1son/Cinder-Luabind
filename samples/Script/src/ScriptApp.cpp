#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace luabind; //included before cinder

#include "CinderLuabind.h"

using ScriptBaseRef = std::shared_ptr< class ScriptBase >;

class ScriptBase {
public:
    ScriptBase( const std::string &name ):mName(name){}
    virtual void setup(){};
    virtual void update(){};
    std::string mName;
    virtual ~ScriptBase(){}
};

class ScriptBaseWrapper : public ScriptBase, public luabind::wrap_base {
    
public:
    
    ScriptBaseWrapper(const std::string &name):ScriptBase(name){}
    
    virtual void update() override
    {
        call<void>("update");
    }
    
    virtual void setup() override
    {
        call<void>("setup");
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


class ScriptApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    lb::ContextRef mLuaContext;
    
};


void ScriptApp::setup()
{
    
    mLuaContext = lb::Context::create("main context");
    
    std::function<void(lua_State*)> scriptBaseBind = [&]( lua_State* state ){
        module(state)
        [
         class_< ScriptBase, ScriptBaseWrapper >("Scriptable")
         .def( luabind::constructor<const std::string&>() )
         .def_readwrite("mName", &ScriptBase::mName)
         .def( "update", &ScriptBase::update, &ScriptBaseWrapper::default_update )
         .def( "setup", &ScriptBase::setup, &ScriptBaseWrapper::default_setup )
         ];
    };
    
    mLuaContext->addBindFunction(scriptBaseBind);
    mLuaContext->bindAll();
    
    mLuaContext->runLuaScript( loadAsset("test.lua") );
    mLuaContext->runLuaScript( loadAsset("test2.lua") );

    
}

void ScriptApp::mouseDown( MouseEvent event )
{
}

void ScriptApp::update()
{
    
    auto script1 = mLuaContext->getGlobal<ScriptBase*>("s");
    auto script2 = mLuaContext->getGlobal<ScriptBase*>("j");
    
    script1->update();
    script2->update();
    
}

void ScriptApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP( ScriptApp, RendererGl )
