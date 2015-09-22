
require "subdir/require_test"

class 'script' (ScriptBase)

function script:__init( name )
    ScriptBase.__init( self, self, name )
    self.inc = 0;
end

function script:setup()

    print( "setup: ", self.mName )

    print(saysomething)				--variable 'saysomething' loaded from require

	manager:clear()
    manager:addCounter( Counter.create(0,1) )
	manager:addCounter( Counter.create(0,10) )
	manager:addCounter( Counter.create(0,100) )

    self:connectFunction( "testPrint" ) --connect testPrint function to the signal on the cider side

end

function script:update()

    manager:incCounters()
    manager:printCounters()

end

function script:testPrint( inc )
    print("counting :", inc)
end

mCounterScript1 = script( "Counter Script 1" )