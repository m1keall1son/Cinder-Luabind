class 'script' (ScriptBase)

function script:__init( name )

    ScriptBase.__init( self, name )		--init the base class
    self.inc = 0.0						--create desired member variables

end

function script:setup()

    print( "setup: ", self.mName )

	mObjectManager:clear()				--if the script was reloaded, clear the manager

	--based on the c++ interface, the MovableObject class can be constructed with either a lua set or a ci.vec3

	local init = { ["scale"] = ci.vec3(0.7), ["translation"] = ci.vec3( -0.5, 0.0, 0.0 ) }
    mObjectManager:addObject( MovableObject( init ) )

	--mObjectManager:addObject( MovableObject( ci.vec3( 0.0 ) ) )

end

function script:update( time )

    self.inc = time

    local obj = mObjectManager:getObject(0)	--grab the first object in the manager

    local mat = ci.translate( ci.vec3( math.cos( self.inc * 0.1 ), 0.0, math.sin( self.inc * 0.1 ) ) )
    mat = mat * ci.rotate( self.inc, ci.vec3(1.0) )
    mat = mat * ci.scale( ci.vec3( math.sin( self.inc * 0.1 ) ) )

    obj:setMatrix( mat )

end

--instantiate this script as a global in the lua context so the app can get it an call its functions

mMovableObjectTransformScript = script( "MovableObjectTransformScript" )