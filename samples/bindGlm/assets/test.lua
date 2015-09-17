class 'script' (Scriptable)

function script:__init( name )
    Scriptable.__init( self, name )
    self.inc = 0.0
end

function script:setup()
    print( "setup: ", self.mName )

    manager:clear()

    local init = { ["scale"] = ci.vec3(0.7), ["translation"] = ci.vec3( -0.5,0.0,0.0 ) }

    manager:addObject( Object( init ) )
    manager:addObject( Object( ci.vec3( 0.0 ) ) )
    manager:addObject( Object( ci.vec3( 0.0 ) ) )

end

function script:update()
    self.inc = self.inc + 1.0

    local obj = manager:getObject(0)
    local mat = obj:getMatrix()

    mat = ci.translate( ci.vec3( Math.cos( self.inc * 0.1 ), 0.0, Math.sin( self.inc * 0.1 ) ) )
    mat = mat * ci.rotate( self.inc, ci.vec3(1.0) )
    --mat = mat * ci.scale( ci.vec3( Math.sin( self.inc * 0.1 ) ) )

    obj:setMatrix( mat )

    local obj2 = manager:getObject(1)
    local mat2 = obj2:getMatrix()

    mat2 = ci.translate( ci.vec3( Math.cos( self.inc * 0.01 ), Math.sin( self.inc * 0.01), 0.0 ) )
    mat2 = mat2 * ci.rotate( self.inc*0.3, ci.vec3(1.0, 0.5, 1.0) )

    obj2:setMatrix( mat2 )

	local obj3 = manager:getObject(2)
	local mat3 = obj3:getMatrix()

	mat3 = ci.translate( ci.vec3(2.0* Math.cos( self.inc * 0.01 ), Math.sin( self.inc * 0.01), 0.0 ) )
	mat3 = mat3 * ci.rotate( self.inc*0.3, ci.vec3(1.0, 0.5, 1.0) )

	obj3:setMatrix( mat3 )

end

s = script( "glm test script" )