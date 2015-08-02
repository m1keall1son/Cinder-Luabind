#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Signals.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace luabind; //included before cinder

#include "CinderLuabind.h"

template<class T>
T* get_pointer(std::weak_ptr<T>& p)
{
    return p.lock().get();
}

template<class T>
T* get_pointer(std::shared_ptr<T>& p)
{
    return p.get();
}

template<class A>
std::shared_ptr<const A>*
get_const_holder(std::shared_ptr<A>*)
{
    return 0;
}

template<class A>
std::weak_ptr<const A>*
get_const_holder(std::weak_ptr<A>*)
{
    return 0;
}

using ScriptBaseRef = std::shared_ptr< class ScriptBase >;

static ci::signals::Signal<void(int)> sSignal;

struct LuaFunctionHandler
{
    LuaFunctionHandler( luabind::object* self, const luabind::object& obj)
    : m_func(obj),_self(self)
    {
        if (luabind::type(obj) != LUA_TFUNCTION) {
            throw std::invalid_argument("invalid lua object");
        }
    }
    
    void operator()(int i)
    {
        try{
            luabind::call_function<void>(m_func, *_self, i);
        }catch( const luabind::error &e )
        {
            console() << e.what() << " : " << lua_tostring(e.state(), -1) << endl;
        }
    }
    
    // ... bunch of more overloads for operator()
    
private:
    luabind::object m_func;
    luabind::object* _self;
};


class ScriptBase {
public:
    ScriptBase( const luabind::object &self, const std::string &name ):mName(name), _self(self){}
    virtual void setup(){};
    virtual void update(){};
    std::string mName;
    virtual void connectFunction( const std::string &name ){
        mFunctions.push_back(LuaFunctionHandler( &_self, _self[name] ));
        std::function<void(int)> fn = std::bind(&LuaFunctionHandler::operator(), &mFunctions.back(), std::placeholders::_1);
        sSignal.connect(fn);
    }
    
    virtual ~ScriptBase(){}
    std::vector<LuaFunctionHandler> mFunctions;
    
private:
    luabind::object _self;
    
};

class ScriptBaseWrapper : public ScriptBase, public luabind::wrap_base {
    
public:
    
    ScriptBaseWrapper( const luabind::object &self, const std::string &name):ScriptBase(self,name){}
    
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

using CounterRef = std::shared_ptr<class Counter>;

class Counter {
public:
    
    static CounterRef create( int starting, int inc ){ return CounterRef( new Counter( starting, inc ) ); }
    
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

using CounterWeakRef = std::weak_ptr<Counter>;

class CounterManager {
public:
    
    void addCounter( const CounterRef& counter ){ mCounters.push_back(counter); mCounters.back()->setName( "counter " + to_string(mCounters.size()) );  }
    CounterRef getCounter( int index ){
	   return mCounters[index];
    }
    void incCounters(){ for(auto&c:mCounters)c->inc(); }
    void decCounters(){ for(auto&c:mCounters)c->dec(); }
    void printCounters(){ for(auto&c:mCounters)c->print(); }
    void removeCounter( int index){ auto it = mCounters.begin(); std::advance( it , index); mCounters.erase( it ); }

private:
    std::vector<CounterRef> mCounters;
};

class ScriptApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    lb::ContextRef mLuaContext;
    CounterManager mCounterManager;
    ScriptBase* mScript1;
    ScriptBase* mScript2;
    int mInc;
};


void ScriptApp::setup()
{
    
    mLuaContext = lb::Context::create("main context");
    
    std::function<void(lua_State*)> scriptBaseBind = [&]( lua_State* state ){
        module(state)
        [
         class_< ScriptBase, ScriptBaseWrapper >("Scriptable")
         .def( luabind::constructor<const luabind::object&,const std::string&>() )
         .def_readwrite("mName", &ScriptBase::mName)
         .def("connectFunction", &ScriptBase::connectFunction)
         .def( "update", &ScriptBase::update, &ScriptBaseWrapper::default_update )
         .def( "setup", &ScriptBase::setup, &ScriptBaseWrapper::default_setup )
         ];
    };
    
    std::function<void(lua_State*)> counterBind = [&]( lua_State* state ){
        module(state)
        [
         
         class_< Counter, CounterRef >("Counter")
         .def( luabind::constructor<int, int>() )
         .def( "inc", &Counter::inc )
         .def( "dec", &Counter::dec )
         .def( "print", &Counter::print )
         .scope[
                def( "create", &Counter::create )
         ],
         class_< CounterManager >("CounterManager")
         .def( luabind::constructor<>() )
         .def( "incCounters", &CounterManager::incCounters )
         .def( "decCounters", &CounterManager::decCounters )
         .def( "printCounters", &CounterManager::printCounters )
	    .def( "getCounter", &CounterManager::getCounter )
         .def( "addCounter", &CounterManager::addCounter )
         .def( "removeCounter", &CounterManager::removeCounter )
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
    
    mScript1 = mLuaContext->getGlobal<ScriptBase*>("s");
    mScript2 = mLuaContext->getGlobal<ScriptBase*>("j");
    
    mScript1->setup();
    mScript2->setup();
    
    mInc = 0;
    
}

void ScriptApp::mouseDown( MouseEvent event )
{
}

void ScriptApp::update()
{

    mInc++;
    sSignal.emit(mInc);
    mScript1->update();
    mScript2->update();
    
}

void ScriptApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP( ScriptApp, RendererGl )
