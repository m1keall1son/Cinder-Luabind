class 'script' (Scriptable)

function script:__init( name )
    super( name )
end

function script:setup()
    print( "setup: ", self.mName )
    c = Counter(0,1)
    manager:addCounter( c )
    c2 = Counter(0,10)
    manager:addCounter( c2 )
end

function script:update()
    --print( self.mName, " counter: ", self.counter )
    manager:incCounters()
    local c = manager:getCounter(0)
    c:inc()
    manager:printCounters()
end

s = script( "a_script" )