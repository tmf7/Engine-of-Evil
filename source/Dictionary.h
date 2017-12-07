#ifndef EVIL_DICTIONARY_H
#define EVIL_DICTIONARY_H

#include "Definitions.h"
#include "Vector.h"

//*************************************************
//				eDictionary
// uses std::unordered_map to store key-values pairs
// of std::strings that can be reinterpreted as
// types [int|float|bool|Vec2|Vec3|Vec4|string]
// this class is primarily designed to load and 
// initaialize eEntitiy-type objects 
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

#endif /* EVIL_DICTIONARY_H */