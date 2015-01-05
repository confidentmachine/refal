#pragma once

#include <map>
#include <string>
#include <Refal2.h>

namespace Refal2 {

const int InvalidLabel = -1;

const int DefaultInitialLabelTableSize = 1024;

class CLabelTable {
public:
	CLabelTable(int initialTableSize = DefaultInitialLabelTableSize);
	~CLabelTable();

	TLabel AddLabel(const std::string& labelText);
	const std::string& GetLabelText(TLabel label);
	CFunction* GetLabelFunction(TLabel label);

	int GetNumberOfLabels() const { return tableFirstFree; }

	TLabel GetFirstLabel() const;
	TLabel GetNextLabel(TLabel afterLabel) const;

private:
	CLabelTable(const CLabelTable&);
	CLabelTable& operator=(const CLabelTable&);

	typedef std::map<std::string, TLabel> TLabelMap;

	struct CLabelInfo {
		CFunction function;
		TLabelMap::const_iterator labelPtr;

		CLabelInfo(const TLabelMap::const_iterator& _labelPtr):
			labelPtr(_labelPtr) {}
			
	};

	void alloc();
	void grow(const TLabelMap::const_iterator& labelPtr);

	TLabelMap labelMap;
	int tableSize;
	int tableFirstFree;
	CLabelInfo* table;
};

// declaration of global label table
extern CLabelTable LabelTable;

} // end of namespace Refal2