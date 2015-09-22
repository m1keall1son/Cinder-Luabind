class 'script' (ScriptBase)

function script:__init( name )
    ScriptBase.__init( self, self, name )
    self.counter = 0
end

function script:setup()
    print( "setup: ", self.mName )
    sayHi()							--any function previously loaded by a require is available to all scripts following it
end

function script:update()
    self.counter = self.counter + 5
    print( self.mName, " my counter: ", self.counter )
end

mCounterScript2 = script( "Counter Script 2" )