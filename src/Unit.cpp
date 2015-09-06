#include <Refal2.h>

namespace Refal2 {

//-----------------------------------------------------------------------------
// CPrintHelper

void CPrintHelper::Variable( std::ostream& outputStream,
	const TVariableIndex variable ) const
{
	outputStream << "V" << variable;
}

void CPrintHelper::Label( std::ostream& outputStream,
	const TLabel label ) const
{
	outputStream << "L:";
	if( PrintLabelWithModule() ) {
		outputStream << ( label / LabelMask ) << ":";
	}
	outputStream << ( label % LabelMask );
}

//-----------------------------------------------------------------------------
// CUnit

bool CUnit::IsEqualWith( const CUnit& unit ) const
{
	if( type != unit.type ) {
		return false;
	}
	switch( type ) {
		case UT_Char:
			return ( c == unit.c );
			break;
		case UT_Label:
			return ( label == unit.label );
			break;
		case UT_Number:
			return ( number == unit.number );
			break;
		case UT_Variable:
			return ( variable == unit.variable );
			break;
		case UT_LeftParen:
		case UT_RightParen:
		case UT_LeftBracket:
		case UT_RightBracket:
			return true;
		default:
			break;
	}
	assert( false );
	return false;
}

void CUnit::Print( std::ostream& outputStream,
	const CPrintHelper& printHelper ) const
{
	switch( GetType() ) {
		case UT_Char:
			outputStream << "'" << Char() << "' ";
			break;
		case UT_Label:
			outputStream << "/";
			printHelper.Label( outputStream, Label() );
			outputStream << "/ ";
			break;
		case UT_Number:
			outputStream << "/" << Number() << "/ ";
			break;
		case UT_Variable:
			printHelper.Variable( outputStream, Variable() );
			outputStream << " ";
			break;
		case UT_LeftParen:
			outputStream << "( ";
			break;
		case UT_RightParen:
			outputStream << ") ";
			break;
		case UT_LeftBracket:
			outputStream << "< ";
			break;
		case UT_RightBracket:
			outputStream << "> ";
			break;
		default:
			assert( false );
			break;
	}
}

//-----------------------------------------------------------------------------
// CUnitList

void CUnitList::Print( const CUnitNode* begin, const CUnitNode* end,
	std::ostream& outputStream,	const CPrintHelper& printHelper )
{
	assert( begin != nullptr );
	assert( end != nullptr );
	const CUnitNode* node = begin;
	while( node != end ) {
		node->Print( outputStream, printHelper );
		node = node->Next();
		assert( node != nullptr );
	}
	node->Print( outputStream, printHelper );
}

void CUnitList::Print( std::ostream& outputStream,
	const CPrintHelper& printHelper ) const
{
	if( !IsEmpty() ) {
		Print( GetFirst(), GetLast(), outputStream, printHelper );
	}
}

void CUnitList::HandyPrint( std::ostream& outputStream,
	const CPrintHelper& printHelper ) const
{
	bool lastWasChar = false;
	for( const CUnitNode* node = GetFirst(); node != 0; node = node->Next() ) {
		if( node->GetType() == UT_Char ) {
			if( !lastWasChar ) {
				outputStream << "'";
				lastWasChar = true;
			}
		} else {
			if( lastWasChar ) {
				outputStream << "'";
				lastWasChar = false;
			}
		}
		switch( node->GetType() ) {
			case UT_Char:
				outputStream << node->Char();
				break;
			case UT_Label:
				outputStream << "/";
				printHelper.Label( outputStream, node->Label() );
				outputStream << "/";
				break;
			case UT_Number:
				outputStream << "/" << node->Number() << "/";
				break;
			case UT_LeftParen:
				outputStream << "(";
				break;
			case UT_RightParen:
				outputStream << ")";
				break;
			case UT_LeftBracket:
				outputStream << "<";
				break;
			case UT_RightBracket:
				outputStream << ">";
				break;
			case UT_Variable:
			default:
				assert( false );
				break;
		}
	}
	if( lastWasChar ) {
		outputStream << "'";
	}
	outputStream << std::endl;
}

void CUnitList::StrangePrint( std::ostream& outputStream,
	const CPrintHelper& printHelper ) const
{
	for( const CUnitNode* node = GetFirst(); node != 0; node = node->Next() ) {
		switch( node->GetType() ) {
			case UT_Char:
				outputStream<< node->Char();
				break;
			case UT_Label:
				outputStream << "'";
				printHelper.Label( outputStream, node->Label() );
				outputStream << "'";
				break;
			case UT_Number:
				std::cout << "'" << node->Number() << "'";
				break;
			case UT_LeftParen:
				std::cout << "(";
				break;
			case UT_RightParen:
				std::cout << ")";
				break;
			case UT_Variable:
			case UT_LeftBracket:
			case UT_RightBracket:
			default:
				assert( false );
				break;
		}
	}
	std::cout << std::endl;
}

void CUnitList::Duplicate( const CUnitNode* fromNode,
	const CUnitNode* toNode, CUnitNode*& fromNodeCopy, CUnitNode*& toNodeCopy )
{
	CNodeList::Duplicate( fromNode, toNode, fromNodeCopy, toNodeCopy );
	CUnitNode* source = const_cast<CUnitNode*>( fromNode );
	CUnitNode* dest = fromNodeCopy;
	while( true ) {
		assert( !source->IsLeftBracket() && !source->IsRightBracket() );
		if( source->IsLeftParen() || source->IsRightParen() ) {
			assert( source->GetType() == dest->GetType() );
			source->PairedParen() = dest;
		}
		if( source == toNode ) {
			assert( dest == toNodeCopy );
			break;
		}
		source = source->Next();
		dest = dest->Next();
	}
	source = const_cast<CUnitNode*>( fromNode );
	while( true ) {
		if( source->IsLeftParen() ) {
			CUnitNode* destLeftParen = source->PairedParen();
			assert( destLeftParen->IsLeftParen() );
			CUnitNode* sourceRightParen = destLeftParen->PairedParen();
			assert( sourceRightParen->IsRightParen() );
			CUnitNode* destRightParen = sourceRightParen->PairedParen();
			assert( destRightParen->IsRightParen() );
			// correct source list
			source->PairedParen() = sourceRightParen;
			sourceRightParen->PairedParen() = source;
			// correct duplicated list
			destLeftParen->PairedParen() = destRightParen;
			destRightParen->PairedParen() = destLeftParen;
		}
		if( source == toNode ) {
			break;
		}
		source = source->Next();
	}
}

//-----------------------------------------------------------------------------

} // end of namespace Refal2
