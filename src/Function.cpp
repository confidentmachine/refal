#include <stack>
#include <Refal2.h>

namespace Refal2 {

void PrintRule(const CFunctionRule* rule)
{
	printf("\t");
	if( rule->isRightDirection ) {
		printf("R");
	}
	printf(" ");
	PrintUnitList( rule->leftPart );
	printf("= ");
	PrintUnitList( rule->rightPart );
}

void PrintFunction(const CFunctionRule* firstRule)
{
	for( ; firstRule != 0; firstRule = firstRule->nextRule ) {
		PrintRule( firstRule );
		printf("\n");
	}
}

CFunctionBuilder::CFunctionBuilder(IFunctionBuilderListener* listener):
	CListener(listener),
	state(FBS_Direction),
	errors(false),
	isRightDirection(false),
	firstRule(0),
	lastRule(0),
	variablesBuilder( static_cast<IVariablesBuilderListener*>(listener) )
{	
}

void CFunctionBuilder::OnVariablesBuilderError(const TVariablesBuilderErrorCodes)
{
}

void CFunctionBuilder::Reset()
{
	state = FBS_Direction;
	acc.Empty();
	emptyRules();
	variablesBuilder.Reset();
	emptyStack();
}

void CFunctionBuilder::Export(CFunction* function)
{
	assert( state == FBS_Direction );

	if( !HasErrors() ) {
		if( firstRule != 0 ) {
			function->SetParsed( &firstRule );
		} else {
			error( FBEC_ThereAreNoRulesInFunction );
		}
	}
	
	Reset();
}

void CFunctionBuilder::SetErrors()
{
	if( !errors ) {
		emptyRules();
	}
	errors = true;
}

void CFunctionBuilder::AddDirection(bool _isRightDirection)
{
	assert( state == FBS_Direction );
	
	isRightDirection = _isRightDirection;
	state = FBS_Left;
}

void CFunctionBuilder::AddEndOfLeft()
{
	assert( state == FBS_Left );
	
	while( !balanceStack.empty() ) {
		balanceStack.pop();
		error( FBEC_UnclosedLeftParenInLeftPart );
	}
	
	if( HasErrors() ) {
		acc.Empty();
	} else {
		acc.Move( &leftPart );
	}
	
	state = FBS_Right;
}

void CFunctionBuilder::AddEndOfRight()
{
	assert( state == FBS_Right );
	
	while( !balanceStack.empty() ) {
		CUnitNode* unit = balanceStack.top();
		balanceStack.pop();
		error( unit->IsLeftParen() ? FBEC_UnclosedLeftParenInRightPart :
			FBEC_UnclosedLeftBracketInRightPart );
	}
	
	if( HasErrors() ) {
		acc.Empty();
		variablesBuilder.Reset();
	} else {
		addRule();
	}
	
	state = FBS_Direction;
}

void CFunctionBuilder::AddVariable(TVariableType type, TVariableName name,
	CQualifier* qualifier)
{
	assert( state != FBS_Direction );
	
	TVariableIndex index;
	
	if( state == FBS_Left ) {
		index = variablesBuilder.AddLeft( name, type, qualifier );
	} else {
		if( qualifier != 0 ) {
			error( FBEC_IllegalQualifierInRightPart );
		}
		index = variablesBuilder.AddRight( name, type );
	}
	
	if( !HasErrors() ) {
		if( index == InvalidVariableIndex ) {
			SetErrors();
		} else {
			acc.AppendVariable( index );
		}
	}
}

void CFunctionBuilder::AddLeftParen()
{
	assert( state != FBS_Direction );
	
	balanceStack.push( acc.AppendLeftParen() );
}

void CFunctionBuilder::AddRightParen()
{
	assert( state != FBS_Direction );
	
	CUnitNode* leftParen = 0;
	
	if( !balanceStack.empty() ) {
		leftParen = balanceStack.top();
	}
	
	if( leftParen != 0 && leftParen->IsLeftParen() ) {
		leftParen->PairedParen() = acc.AppendRightParen(leftParen);
		balanceStack.pop();
	} else {
		error( FBEC_RightParenDoesNotMatchLeftParen );
	}
}

void CFunctionBuilder::AddLeftBracket()
{
	assert( state != FBS_Direction );

	if( state == FBS_Left ) {
		error( FBEC_IllegalLeftBracketInLeftPart );
	} else {
		balanceStack.push( acc.AppendLeftBracket() );
	}
}

void CFunctionBuilder::AddRightBracket()
{
	assert( state != FBS_Direction );
	
	if( state == FBS_Left ) {
		error( FBEC_IllegalRightBracketInLeftPart );
	} else {
		CUnitNode* leftBracket = 0;
	
		if( !balanceStack.empty() ) {
			leftBracket = balanceStack.top();
		}
		
		if( leftBracket != 0 && leftBracket->IsLeftBracket() ) {
			leftBracket->PairedParen() = acc.AppendRightBracket(leftBracket);
			balanceStack.pop();
		} else {
			error( FBEC_RightBracketDoesNotMatchLeftBracket );
		}
	}
}

void CFunctionBuilder::emptyRules()
{
	if( firstRule != 0 ) {
		while( firstRule != 0 ) {
			CFunctionRule* tmp = firstRule;
			firstRule = firstRule->nextRule;
			delete tmp;
		}
	}
	firstRule = 0;
	lastRule = 0;
}

void CFunctionBuilder::addRule()
{
	CFunctionRule* newRule = new CFunctionRule( isRightDirection );
	leftPart.Move( &(newRule->leftPart) );
	acc.Move( &(newRule->rightPart) );
	variablesBuilder.Export( &(newRule->variables) );
	
	if( firstRule != 0 ) {
		lastRule->nextRule = newRule;
		lastRule = lastRule->nextRule;
	} else {
		firstRule = newRule;
		lastRule = firstRule;
	}
}

} // end of namespace refal2