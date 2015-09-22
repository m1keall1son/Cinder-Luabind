#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Signals.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace luabind; //included before cinder

#include "CinderLuabind.h"

static cinder::signals::Signal< void(int) > sEventSignal;

class ScriptBase {
public:
    ScriptBase( const luabind::object &self, const std::string &name ):mName(name), _self(self){}
    
    virtual void setup(){};
    virtual void update(){};
    
    std::string mName;
    
    //! allows passing of an arbitrary lua member function from the derived class which can then be hooked into a signal or stored for use whenever
    virtual void connectFunction( const std::string &name ){
	   
        mFunctions.push_back( ci::lb::FunctionHandler( &_self, _self[name] ) );
	   
	   std::function<void(int)> fn = std::bind(
									   (void(ci::lb::FunctionHandler::*)(int))&ci::lb::FunctionHandler::operator(), //have to specialize the overload
									   &mFunctions.back(),
									   std::placeholders::_1
									   );
	   
	   mConnections.push_back( sEventSignal.connect(fn) );
	   
    }
    
    virtual ~ScriptBase(){
	   for( auto & connection : mConnections )connection.disconnect();
    }
    
    std::vector<ci::lb::FunctionHandler> mFunctions;
    std::vector< signals::Connection > mConnections;
    
private:
    luabind::object _self;
    
};

//! wrap the base class to allow extending ScriptBase on the lua side

LB_WRAP_BASE_BEGIN(ScriptBase);
LB_WRAP_BASE_CONSTRUCTOR_ARGS(ScriptBase, (const luabind::object &) self, (const std::string &) name );
LB_WRAP_BASE_VIRTUAL_FN(ScriptBase, void, setup);
LB_WRAP_BASE_VIRTUAL_FN(ScriptBase, void, update);
LB_WRAP_BASE_END();


///COUNTING OBJECTS///

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
    void clear(){ mCounters.clear(); }

private:
    std::vector<CounterRef> mCounters;
};


class ScriptApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
     void reloadScripts();
    
    lb::ContextRef mLuaContext;
    CounterManager mCounterManager;
    ScriptBase* mCounterScript1;
    ScriptBase* mCounterScript2;
    int mInc;
};

void ScriptApp::reloadScripts()
{
    try{
	   mLuaContext->runLuaScript( loadAsset("CounterScript1.lua") );
    }catch( const ci::lb::LuaException &e )
    {
	   console() << e.what() << endl;
    }
    
    try{
	   mLuaContext->runLuaScript( loadAsset("CounterScript2.lua") );
    }catch( const ci::lb::LuaException &e )
    {
	   console() << e.what() << endl;
    }
    
    mLuaContext->setGlobal( "manager", &mCounterManager );
    
    mCounterScript1 = mLuaContext->getGlobal<ScriptBase*>("mCounterScript1");
    mCounterScript2 = mLuaContext->getGlobal<ScriptBase*>("mCounterScript2");
    
    mCounterScript1->setup();
    mCounterScript2->setup();
}

void ScriptApp::setup()
{
    
    mLuaContext = ci::lb::Context::create("main context");
    
    luabind::module( mLuaContext->getState() )
    [
         class_< ScriptBase, LB_WRAP_BASE_GET(ScriptBase) >("ScriptBase")
         .def( luabind::constructor<const luabind::object&,const std::string&>() )
         .def_readwrite( "mName", &ScriptBase::mName)
         .def( "connectFunction", &ScriptBase::connectFunction)
         .def( "update", &ScriptBase::update, &LB_WRAP_BASE_GET_DEFAULT_FN(ScriptBase, update) )
         .def( "setup", &ScriptBase::setup, &LB_WRAP_BASE_GET_DEFAULT_FN(ScriptBase, setup) ),
	
	    //! explicitly tell lua to handle Counters as shared pointers inside lua
         class_< Counter, CounterRef >("Counter")
         .def( luabind::constructor<int, int>() )
         .def( "inc", &Counter::inc )
         .def( "dec", &Counter::dec )
         .def( "print", &Counter::print )
	   //! static class functions are declared as a scope on a luabind::class_
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
	    .def( "clear", &CounterManager::clear )

    ];

    mLuaContext->setGlobal( "manager", &mCounterManager );

    reloadScripts();
    
    mInc = 0;
    
}

void ScriptApp::mouseDown( MouseEvent event )
{
    reloadScripts();
}

void ScriptApp::update()
{

    mInc++;
    
    //! any script can create a function that can hook into this event and receive its input
    //! even while the app is running after reloading the script
    sEventSignal.emit(mInc);
    
    mCounterScript1->update();
    mCounterScript2->update();
    
}

void ScriptApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP( ScriptApp, RendererGl )
