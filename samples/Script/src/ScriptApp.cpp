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
        try{
            call<void>("update");
        }catch( const luabind::error &e )
        {
            console() << e.what() << " : " << lua_tostring(e.state(), -1) << endl;
        }
    }
    
    virtual void setup() override
    {
        try{
            call<void>("setup");
        }catch( const luabind::error &e )
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

class Counter {
public:
    Counter( int startingVal, int inc ):mCounter(startingVal), mInc(inc){}
    void inc(){ mCounter+=mInc; }
    void dec(){ mCounter-=mInc; }
    void print(){ console() << mName + " : " +to_string(mCounter) << endl; }
    
private:
    
    void setName( const std::string& name ){ mName = name; }
    
    std::string mName;
    int mCounter;
    int mInc;
    
    friend class CounterManager;
};

class CounterManager {
public:
    
    void addCounter( const Counter& counter ){ mCounters.push_back(counter); mCounters.back().setName( "counter " + to_string(mCounters.size()) );  }
    Counter& getCounter( int index ){
        return mCounters[index];
    }
    void incCounters(){ for(auto&c:mCounters)c.inc(); }
    void decCounters(){ for(auto&c:mCounters)c.dec(); }
    void printCounters(){ for(auto&c:mCounters)c.print(); }

private:
    std::vector<Counter> mCounters;
};

class ScriptApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    lb::ContextRef mLuaContext;
    CounterManager mCounterManager;
    
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
    
    std::function<void(lua_State*)> counterBind = [&]( lua_State* state ){
        module(state)
        [
         class_< Counter >("Counter")
         .def( luabind::constructor<int, int>() )
         .def( "inc", &Counter::inc )
         .def( "dec", &Counter::dec )
         .def( "print", &Counter::print ),
         class_< CounterManager >("CounterManager")
         .def( luabind::constructor<>() )
         .def( "incCounters", &CounterManager::incCounters )
         .def( "decCounters", &CounterManager::decCounters )
         .def( "printCounters", &CounterManager::printCounters )
         .def( "getCounter", ( Counter&(CounterManager::*)(int) )&CounterManager::getCounter )
         .def( "addCounter", ( void(CounterManager::*)(const Counter &) )&CounterManager::addCounter )
         ];
    };
    
    mLuaContext->addBindFunction(scriptBaseBind);
    mLuaContext->addBindFunction(counterBind);
    mLuaContext->bindAll();
    
    try{
        mLuaContext->runLuaScript( loadAsset("test.lua") );
    }catch( const lb::LuaCompileException &e )
    {
        console() << e.what() << endl;
    }
    
    try{
        mLuaContext->runLuaScript( loadAsset("test2.lua") );
    }catch( const lb::LuaCompileException &e )
    {
        console() << e.what() << endl;
    }
    
    luabind::globals(mLuaContext->getState())["manager"] = &mCounterManager;
    
    auto script1 = mLuaContext->getGlobal<ScriptBase*>("s");
    auto script2 = mLuaContext->getGlobal<ScriptBase*>("j");
    
    script1->setup();
    script2->setup();
    
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
