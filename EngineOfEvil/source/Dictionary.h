/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_DICTIONARY_H
#define EVIL_DICTIONARY_H

#include "Definitions.h"
#include "Vector.h"

namespace evil {

//*************************************************
//				eDictionary
// uses std::unordered_map to store key-values pairs
// of std::strings that can be reinterpreted as
// types [int|float|bool|Vec2|Vec3|Vec4|string]
// this class is primarily designed to load and 
// initaialize eEntity-type objects 
//*************************************************
class eDictionary {
public:

	bool												IsEmpty() const { return args.empty(); }
	void												Clear();
	void												Remove( const char *key );
	void												Set( const char *key, const char *value );
	void												SetFloat( const char *key, float val );
	void												SetInt( const char *key, int val );
	void												SetBool( const char *key, bool val );
	void												SetVec2( const char *key, const eVec2 & value );
	void												SetVec3( const char *key, const eVec3 & value );
	void												SetVec4( const char *key, const eQuat & value );

														// these return default values of 0.0, 0 and false (based off "")
	const char *										GetString( const char *key, const char *defaultString = "" ) const;
	float												GetFloat( const char *key, const char *defaultString ) const;
	int													GetInt( const char *key, const char *defaultString ) const;
	bool												GetBool( const char *key, const char *defaultString ) const;
	eVec2												GetVec2( const char *key, const char *defaultString = nullptr ) const;
	eVec3												GetVec3( const char *key, const char *defaultString = nullptr) const;
	eQuat												GetVec4( const char *key, const char *defaultString = nullptr) const;


	typedef std::unordered_map<std::string, std::string>::iterator iterator;
    typedef std::unordered_map<std::string, std::string>::const_iterator const_iterator;

    iterator											begin()				{ return args.begin(); }
    const_iterator										begin() const		{ return args.begin(); }
    iterator											end()				{ return args.end(); }
    const_iterator										end() const			{ return args.end(); }

private:

	std::unordered_map<std::string, std::string>		args;
	std::array<char, MAX_ESTRING_LENGTH>				setBuffer;
};

//********************
// eDictionary::Clear
//********************
inline void eDictionary::Clear() {
	args.clear();
	setBuffer.fill(0);
}

//********************
// eDictionary::Remove
//********************
inline void eDictionary::Remove( const char *key ) {
	if ( key == NULL || key[0] == '\0' )
		return;

	const auto & iter = args.find(key);
	if ( iter != args.end() )
		args.erase(key);
}

//********************
// eDictionary::Set
//********************
inline void eDictionary::Set( const char *key, const char *value ) {
	if ( key == NULL || key[0] == '\0' )
		return;

	args[key] = value;
}

//********************
// eDictionary::SetFloat
//********************
inline void eDictionary::SetFloat( const char *key, float val ) {
	snprintf(setBuffer.data(), setBuffer.size(), "%f", val);
	Set( key, setBuffer.data() );
}

//********************
// eDictionary::SetInt
//********************
inline void eDictionary::SetInt( const char *key, int val ) {
	snprintf(setBuffer.data(), setBuffer.size(), "%i", val);
	Set( key, setBuffer.data() );
}

//********************
// eDictionary::SetBool
//********************
inline void eDictionary::SetBool( const char *key, bool val ) {
	snprintf(setBuffer.data(), setBuffer.size(), "%i", val);
	Set( key, setBuffer.data() );
}

//********************
// eDictionary::GetString
//********************
inline const char * eDictionary::GetString( const char *key, const char *defaultString ) const {
	const auto & iter = args.find(key);
	if ( iter != args.end() )
		return iter->second.c_str();

	return defaultString;
}

//********************
// eDictionary::GetFloat
//********************
inline float eDictionary::GetFloat( const char *key, const char *defaultString ) const {
	return (float)atof( GetString( key, defaultString ) );
}

//********************
// eDictionary::GetInt
//********************
inline int eDictionary::GetInt( const char *key, const char *defaultString ) const {
	return atoi( GetString( key, defaultString ) );
}

//********************
// eDictionary::GetBool
//********************
inline bool eDictionary::GetBool( const char *key, const char *defaultString ) const {
	return ( atoi( GetString( key, defaultString ) ) != 0 );
}

//********************
// eDictionary::SetVec2
//********************
inline void eDictionary::SetVec2( const char *key, const eVec2 & value ) {
	snprintf(setBuffer.data(), setBuffer.size(), "%f %f", value.x, value.y);
	Set( key, setBuffer.data() );
}

//********************
// eDictionary::SetVec3
//********************
inline void eDictionary::SetVec3( const char *key, const eVec3 & value ) {
	snprintf(setBuffer.data(), setBuffer.size(), "%f %f %f", value.x, value.y, value.z);
	Set( key, setBuffer.data() );
}

//********************
// eDictionary::SetVec4
//********************
inline void eDictionary::SetVec4( const char *key, const eQuat & value ) {
	snprintf(setBuffer.data(), setBuffer.size(), "%f %f %f %f", value.x, value.y, value.z, value.w);
	Set( key, setBuffer.data()  );
}

}      /* evil */
#endif /* EVIL_DICTIONARY_H */