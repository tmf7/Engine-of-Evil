#include "Dictionary.h"

//********************
// eDictionary::GetVec2
//********************
eVec2 eDictionary::GetVec2( const char *key, const char *defaultString ) const {
	eVec2			result;
	const char *	resultString;
	
	if (!defaultString)
		defaultString = "0 0";

	resultString = GetString(key, defaultString);
	sscanf_s(resultString, "%f %f", &result.x, &result.y);
	return result;
}

//********************
// eDictionary::GetVec3
//********************
eVec3 eDictionary::GetVec3( const char *key, const char *defaultString) const {
	eVec3		 result;
	const char * resultString;
	
	if (!defaultString)
		defaultString = "0 0 0";

	resultString = GetString(key, defaultString);
	sscanf_s(resultString, "%f %f %f", &result.x, &result.y, &result.z);
	return result;
}

//********************
// eDictionary::GetVec4
//********************
eQuat eDictionary::GetVec4( const char *key, const char *defaultString) const {
	eQuat		 result;
	const char * resultString;
	
	if (!defaultString)
		defaultString = "0 0 0 0";

	resultString = GetString( key, defaultString );
	sscanf_s(resultString, "%f %f %f %f", &result.x, &result.y, &result.z, &result.w);
	return result;
}