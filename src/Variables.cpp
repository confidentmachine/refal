#include <Refal2.h>

namespace Refal2 {
	
void CVariables::Reset()
{
	if( variables != 0 ) {
		::operator delete( variables );
		variables = 0;
	}
	variablesSize = 0;

	if( variablesValues != 0 ) {
		::operator delete( variablesValues );
		variablesValues = 0;
	}
	variablesValuesSize = 0;
}

void CVariables::Swap( CVariables& swapWith )
{
	std::swap( swapWith.variables, variables );
	std::swap( swapWith.variablesSize, variablesSize );
	std::swap( swapWith.variablesValues, variablesValues );
	std::swap( swapWith.variablesValuesSize, variablesValuesSize );
}

bool CVariables::IsFull( TVariableIndex variableIndex ) const
{
	assert( IsValidVariableIndex( variableIndex ) );
	CVariable& variable = variables[variableIndex];
	return ( variable.position == variable.topPosition );
}

bool CVariables::IsSet( TVariableIndex variableIndex ) const
{
	assert( IsValidVariableIndex( variableIndex ) );
	CVariable& variable = variables[variableIndex];
	return ( variable.position > variable.originPosition );
}

void CVariables::Set( TVariableIndex variableIndex, TTableIndex tableIndex )
{
	assert( IsValidVariableIndex( variableIndex ) );
	CVariable& variable = variables[variableIndex];
	if( variable.position < variable.topPosition ) {
		allocVariablesValues();
		variablesValues[variable.position] = tableIndex;
		variable.position++;
	}
}

TTableIndex CVariables::GetMainValue( TVariableIndex variableIndex ) const
{
	assert( IsSet( variableIndex ) );
	return variablesValues[variables[variableIndex].originPosition];
}

bool CVariables::Get( TVariableIndex variableIndex, TTableIndex& tableIndex)
{
	assert( IsValidVariableIndex( variableIndex ) );
	assert( variablesValues != 0 );
	CVariable& variable = variables[variableIndex];
	if( variable.position > variable.originPosition ) {
		variable.position--;
		tableIndex = variablesValues[variable.position];
		return true;
	} else /* var.position == var.originPosition */ {
		tableIndex = variablesValues[variable.position];
		return false;
	}
}

CVariablesBuilder::CVariablesBuilder( IVariablesBuilderListener* listener ):
	CListener( listener )
{
	Reset();
}

void CVariablesBuilder::Reset()
{
	countOfVariables = 0;
	for( int i = 0; i < VariablesInfoSize; ++i ) {
		variables[i].type = InvalidVariableType;
		variables[i].qualifier.Empty();
	}
}

void CVariablesBuilder::Export( CVariables& v )
{
	v.Reset();
	if( countOfVariables > 0 ) {
		v.variablesSize = countOfVariables;
		v.variables = static_cast<CVariable*>(
			::operator new( v.variablesSize * sizeof( CVariable ) ) );
		v.variablesValuesSize = 0;
		for( int i = 0; i < VariablesInfoSize; i++ ) {
			CVariableInfo& vi = variables[i];
			if( vi.type != InvalidVariableType ) {
				int countOfValues = std::min( vi.countLeft, vi.countRight );
				new( v.variables + vi.name )CVariable( vi.type,
					v.variablesValuesSize, countOfValues );
				v.variablesValuesSize += countOfValues;
				vi.qualifier.Move( v.variables[vi.name].qualifier );
			}
		}
	}
	Reset();
}

TVariableIndex CVariablesBuilder::AddLeft( TVariableName name,
	TVariableType type, CQualifier* qualifier )
{
	if( !checkName( name ) ) {
		return InvalidVariableIndex;
	}
	CVariableInfo& var = variables[name];
	if( var.type == InvalidVariableType ) {
		if( checkType( type ) ) {
			var.name = countOfVariables;
			var.type = type;
			var.countLeft = 1;
			var.countRight = 0;
			if( qualifier != 0 ) {
				qualifier->Move( var.qualifier );
			}
			++countOfVariables;
			return var.name;
		}
	} else if( var.type == type ) {
		var.countLeft++;
		if( qualifier != 0 ) {
			var.qualifier.DestructiveIntersection( *qualifier );
		}
		return var.name;
	} else {
		error( VBEC_TypeOfVariableDoesNotMatch );
	}
	return InvalidVariableIndex;
}

TVariableIndex CVariablesBuilder::AddRight( TVariableName name,
	TVariableType type,	CQualifier* qualifier )
{
	if( !checkName( name ) ) {
		return InvalidVariableIndex;
	}
	CVariableInfo& var = variables[name];
	if( var.type == type ) {
		var.countRight++;
		if( qualifier != 0 ) {
			var.qualifier.DestructiveIntersection( *qualifier );
		}
		return var.name;
	} else {
		if( var.type == InvalidVariableType ) {
			error( VBEC_NoSuchVariableInLeftPart );
		} else {
			error( VBEC_TypeOfVariableDoesNotMatch );
		}
		return InvalidVariableIndex;
	}
}

} // end of namespace refal2
