class 'script' (Scriptable)

function script:__init( name )
    super( name )
    self.counter = 0
end

function script:setup()
    print( "setup: ", self.mName )
end

function script:update()
    self.counter = self.counter + 5
    print( self.mName, " counter: ", self.counter )
end

j = script( "other_script" )