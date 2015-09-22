#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderLuabind.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HelloWorldApp : public App {
  public:
	void setup() override;
	void draw() override;
	
	ci::lb::ContextRef mLuaContext;
	std::string mHelloWorld;
	std::string mMessage;
};

const std::string script = {
		"do \n"
		"	helloworld = \"Hello World!\" \n"
		"	print( helloworld ) \n"
		"	print( message ) \n"
		"	printMessage() \n"
		"end"
};

void printMessage(){
	console() << "This came from a c++ function" << endl;
}

void HelloWorldApp::setup()
{
	//! create a lua context
	mLuaContext = lb::Context::create( "main lua context" );
	
	mMessage = "This message came from c++!";
	
	//! set a global variable for lua to use from the c++ app
	mLuaContext->setGlobal( "message", mMessage.c_str() );
	
	//! bind a c++ funtion to lua to use in a script!
	luabind::module( mLuaContext->getState() )[ luabind::def( "printMessage", &printMessage ) ];
	
	//! try catch excpetions while compiling scripts (they can also be loaded from DataSourceRef)
	try{
		mLuaContext->runLuaScript( script );
	}catch( const ci::lb::LuaException &e ){
		CI_LOG_E( e.what() );
	}
	
	//! retrieve global variables defined in your script
	mHelloWorld = mLuaContext->getGlobal<std::string>( "helloworld" );
	
}

void HelloWorldApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::drawString( mHelloWorld, vec2(25,25), Color( 0., 1., 0. ), ci::Font( "Helvetica", 50 ) );
}

CINDER_APP( HelloWorldApp, RendererGl )
