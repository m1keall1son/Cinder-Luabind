
require "subdir/require_test"

class 'script' (Scriptable)

function script:__init( name )
    Scriptable.__init( self, self, name )
    self.inc = 0;
    self.c = nil

end

function script:setup()

    print( "setup: ", self.mName )
    print(saysomething)

    manager:addCounter( Counter.create(0,1) )

    self.c = manager:getCounter(0)

    manager:removeCounter(0)

    self:connectFunction( "testPrint" )
end

function script:update()
    --print( self.mName, " counter: ", self.counter )
    manager:incCounters()

    if self.c ~= nil then
        print("this lock is not broken")
        self.c:inc()
    else
        print("this lock is broken")
    end

    manager:printCounters()

end

function script:testPrint( inc )
    print("counting :", inc)
end

s = script( "a_script" )