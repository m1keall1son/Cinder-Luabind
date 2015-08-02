class 'script' (Scriptable)

function script:__init( name )
    Scriptable.__init( self, self, name )
    self.counter = 0
end

function script:setup()
    print( "setup: ", self.mName )
    sayHi()
end

function script:update()
    self.counter = self.counter + 5
    print( self.mName, " counter: ", self.counter )
end

j = script( "other_script" )