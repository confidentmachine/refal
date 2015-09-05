#pragma once

#include <Refal2.h>

namespace Refal2 {

//-----------------------------------------------------------------------------
// CReceptacle

class CReceptacle {
public:
	CReceptacle() {}

	// return true if keyValue is correct (contain '=' at top level)
	bool Burry( CUnitList& keyValue );
	bool Replace( CUnitList& keyValue );
	void DigOut( const CUnitList& key, CUnitList& value );
	void Copy( const CUnitList& key, CUnitList& value );
	void DigOutAll( CUnitList& receptacle );

	static bool IsValidKeyValue( const CUnitList& keyValue );

private:
	CUnitList data;

	void burry( CUnitList& keyValue );
	bool find( const CUnitNode* key, CUnitNode*& leftBorderNode,
		CUnitNode*& equalNode );

	CReceptacle( const CReceptacle& );
	CReceptacle& operator=( const CReceptacle& );
};

//-----------------------------------------------------------------------------

} // end of namespace Refal2
