//
//  CinderBindings.h
//  bindGlm
//
//  Created by Mike Allison on 9/19/15.
//
//

#pragma once

#define LB_CI_VEC2()\
class_<ci::vec2>("vec2")\
.def( constructor<float>() )\
.def( constructor<float, float>() )\
.def( constructor<const ci::vec2 &>() )\
.def_readwrite("x", &ci::vec2::x)\
.def_readwrite("y", &ci::vec2::y)\
.def( self + other<const ci::vec2 &>() )\
.def( self - other<const ci::vec2 &>() )\
.def( self / other<const ci::vec2 &>() )\
.def( self * other<const ci::vec2 &>() )\
.def( self == other<const ci::vec2 &>() )\
.def( tostring(self) )

#define LB_CI_IVEC2()\
class_<ci::ivec2>("ivec2")\
.def( constructor<int>() )\
.def( constructor<int, int>() )\
.def( constructor<const ci::ivec2 &>() )\
.def_readwrite("x", &ci::ivec2::x)\
.def_readwrite("y", &ci::ivec2::y)\
.def( self + other<const ci::ivec2 &>() )\
.def( self - other<const ci::ivec2 &>() )\
.def( self / other<const ci::ivec2 &>() )\
.def( self * other<const ci::ivec2 &>() )\
.def( self == other<const ci::ivec2 &>() )\
.def( tostring(self) )

#define LB_CI_VEC3()\
class_<ci::vec3>("vec3")\
.def( constructor<float, float, float>() )\
.def( constructor<const ci::vec3 &>() )\
.def( constructor<float>() )\
.def_readwrite("x", &ci::vec3::x)\
.def_readwrite("y", &ci::vec3::y)\
.def_readwrite("z", &ci::vec3::z)\
.def( self + other<const ci::vec3 &>() )\
.def( self - other<const ci::vec3 &>() )\
.def( self / other<const ci::vec3 &>() )\
.def( self * other<const ci::vec3 &>() )\
.def( self * other<const ci::mat3 &>() )\
.def( self == other<const ci::vec3 &>() )\
.def( tostring(self) )

#define LB_CI_VEC4()\
class_<ci::vec4>("vec4")\
.def( constructor<float, float, float, float>() )\
.def( constructor<const ci::vec4 &>() )\
.def( constructor<float>() )\
.def_readwrite("x", &ci::vec4::x)\
.def_readwrite("y", &ci::vec4::y)\
.def_readwrite("z", &ci::vec4::z)\
.def_readwrite("w", &ci::vec4::w)\
.def( self + other<const ci::vec4 &>() )\
.def( self - other<const ci::vec4 &>() )\
.def( self / other<const ci::vec4 &>() )\
.def( self * other<const ci::vec4 &>() )\
.def( self * other<const ci::mat4 &>() )\
.def( self * other<const ci::quat &>() )\
.def( self == other<const ci::vec4 &>() )\
.def( tostring(self) )

#define LB_CI_QUAT()\
class_<ci::quat>("quat")\
.def( constructor<float, float, float, float>() )\
.def( constructor<const ci::quat &>() )\
.def_readwrite("x", &ci::quat::x)\
.def_readwrite("y", &ci::quat::y)\
.def_readwrite("z", &ci::quat::z)\
.def_readwrite("w", &ci::quat::w)\
.def( self + other<const ci::quat &>() )\
.def( tostring(self) )\

#define LB_CI_MAT4()\
class_<ci::mat4>("mat4")\
.def( constructor<const ci::vec4&, const ci::vec4&, const ci::vec4&, const ci::vec4&>() )\
.def( constructor<>() )\
.def( constructor<float>() )\
.def( constructor<const ci::mat4&>() )\
.def( self + other<const ci::mat4 &>() )\
.def( self - other<const ci::mat4 &>() )\
.def( self / other<const ci::mat4 &>() )\
.def( self * other<const ci::mat4 &>() )\
.def( tostring(self) )

#define LB_CI_MAT3()\
class_<ci::mat3>("mat3")\
.def( constructor<const ci::vec3&, const ci::vec3&, const ci::vec3&>() )\
.def( constructor<>() )\
.def( constructor<const ci::mat3&>() )\
.def( self + other<const ci::mat3 &>() )\
.def( self - other<const ci::mat3 &>() )\
.def( self / other<const ci::mat3 &>() )\
.def( self * other<const ci::mat3 &>() )\
.def( tostring(self) )

#define LB_CI_GLM_TYPES()\
	LB_CI_VEC2(),\
	LB_CI_VEC3(),\
	LB_CI_VEC4(),\
	LB_CI_QUAT(),\
	LB_CI_MAT3(),\
	LB_CI_MAT4()

#define LB_CI_LENGTH()\
def( "length", (float(*)(const ci::vec3 &)) &ci::length ),\
def( "length", (float(*)(const ci::quat &)) &ci::length )

#define LB_CI_DOT()\
def( "dot", ( float(*)( const ci::vec3 &, const ci::vec3 & ) )&ci::dot ),\
def( "dot", ( float(*)( const ci::vec2 &, const ci::vec2 & ) )&ci::dot ),\
def( "dot", ( float(*)( const ci::vec4 &, const ci::vec4 & ) )&ci::dot )

#define LB_CI_NORMALIZE()\
def( "normalize", ( ci::vec3(*)( const ci::vec3 &) )&ci::normalize ),\
def( "normalize", ( ci::vec2(*)( const ci::vec2 &) )&ci::normalize ),\
def( "normalize", ( ci::vec4(*)( const ci::vec4 &) )&ci::normalize ),\
def( "normalize", ( float(*)( const float &) )&ci::normalize )

#define LB_CI_CROSS()\
def( "normalize", ( ci::vec3(*)( const ci::vec3 &, const ci::vec3 &) )&ci::cross )

#define LB_CI_GLM_UTILS()\
	LB_CI_LENGTH(),\
	LB_CI_DOT(),\
	LB_CI_NORMALIZE(),\
	LB_CI_CROSS()

#define LB_CI_TRANSLATE()\
def( "translate", ( ci::mat4(*)( const ci::mat4 &, const ci::vec3 & ) )&ci::translate ),\
def( "translate", ( ci::mat4(*)( const ci::vec3 & ) )&ci::translate )

#define LB_CI_SCALE()\
def( "scale", ( ci::mat4(*)( const ci::mat4 &, const ci::vec3 & ) )&ci::scale ),\
def( "scale", ( ci::mat4(*)( const ci::vec3 & ) )&ci::scale )

#define LB_CI_ROTATE()\
def( "rotate", ( ci::mat4(*)( float, const ci::vec3 & ) )&ci::rotate ),\
def( "rotate", ( ci::mat4(*)( const ci::mat4&, float, const ci::vec3 & ) )&ci::rotate ),\
def( "rotate", ( ci::vec3(*)( const ci::quat&, const ci::vec3 & ) )&ci::rotate ),\
def( "rotate", ( ci::vec4(*)( const ci::quat&, const ci::vec4 & ) )&ci::rotate )

#define LB_CI_GLM_TRANSFORMS()\
	LB_CI_LENGTH(),\
	LB_CI_TRANSLATE(),\
	LB_CI_SCALE(),\
	LB_CI_ROTATE()

#define LB_CI_GLM()\
	LB_CI_GLM_TYPES(),\
	LB_CI_GLM_UTILS(),\
	LB_CI_GLM_TRANSFORMS()