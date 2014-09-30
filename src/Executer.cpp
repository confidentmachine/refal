#include "Executer.h"

#include "FieldOfView.h"
#include "Operation.h"
#include "Qualifier.h"
#include "Common.h"

namespace Refal2 {

CExecuter::CExecuter():
	stack_size(0), stack(0), table_size(0), table(0)
{
}

CExecuter::~CExecuter()
{
	delete[] stack;
	delete[] table;
}

void CExecuter::SetStackSize(int new_stack_size)
{
	if( new_stack_size > stack_size ) {
		delete[] stack;
		stack_size = new_stack_size;
		stack = new CState[stack_size];
	}
}

void CExecuter::SetTableSize(int new_table_size)
{
	if( new_table_size > table_size ) {
		delete[] table;
		table_size = new_table_size;
		table = new CUnitLink*[table_size];
	}
}

const char* names[] = {
	"OT_goto", /* one argument operation pointer */
	"OT_set_next_rule", /* one argument operation pointer */
	"OT_matching_complete", /* no arguments */
	"OT_empty_expression_match", /* no arguments */
	"OT_left_symbol_match", /* one argument symbol */
	"OT_right_symbol_match", /* one argument symbol */
	"OT_left_parens_match", /* no arguments */
	"OT_right_parens_match", /* no arguments */
	"OT_set_pointers", /* two arguments offset in table */
	"OT_left_s_variable_match", /* no arguments */
	"OT_right_s_variable_match", /* no arguments */
	"OT_left_duplicate_s_variable_match", /* one argument offset in table */
	"OT_right_duplicate_s_variable_match", /* one argument offset in table */
	"OT_closed_e_variable_match", /* no arguments */
	"OT_left_duplicate_wve_variable_match", /* one argument offset in table */
	"OT_right_duplicate_wve_variable_match", /* one argument offset in table*/
	"OT_left_w_variable_match", /* no arguments */
	"OT_right_w_variable_match", /* no arguments */
	"OT_check_not_empty", /* no arguments */
	"OT_left_e_variable_match_begin", /* no arguments */
	"OT_left_v_variable_match_begin", /* no arguments */
	"OT_left_ve_variable_match", /* no arguments */
	"OT_left_ve_variable_with_qualifier_match", /* one argument qualifier */
	"OT_right_e_variable_match_begin", /* no arguments */
	"OT_right_v_variable_match_begin", /* no arguments */
	"OT_right_ve_variable_match", /* no arguments */
	"OT_right_ve_variable_with_qualifier_match", /* one argument qualifier */
	"OT_decriase_stack_depth", /* one argument int */
	"OT_check_qualifier_last_s_variable", /* one argument qualifier */
	"OT_check_qualifier_last_wve_variable", /* one argument qualifier */
	"OT_left_max_qualifier", /* one argument qualifier */
	"OT_right_max_qualifier", /* one argument qualifier */
	"OT_insert_symbol", /* one argument symbol */
	"OT_insert_parens", /* no arguments */
	"OT_jump_right_paren", /* no arguments */
	"OT_save_point", /* one argument offset in points array */
	"OT_allocate_points_array", /* one argument points array size */
	"OT_move_s", /* one argument offset in table */
	"OT_copy_s", /* one argument offset in table */
	"OT_move_wve", /* one argument offset in table */
	"OT_copy_wve", /* one argument offset in table */
	"OT_return" /* one argument points array size */
};

void print(COperation* operation)
{
	std::cout << names[operation->Type()] << "\n";
}

void CExecuter::Run(COperation* operation, CUnitLink* first, CUnitLink* last)
{
	lb = first;
	rb = last;
	op = operation;
	try {
		while( true ) {
			print(op);
			switch( op->Type() ) {
				case COperation::OT_goto:
					op = op->Operation()->operation;
					break;
					
				case COperation::OT_set_next_rule:
					next_rule = op->Operation()->operation;
					COperationOperation::Next(op);
					break;

				case COperation::OT_matching_complete:
					lb = first->Prev();
					COperation::Next(op);
					break;

				/* left part operations */
				case COperation::OT_empty_expression_match:
					if( lb->Next() == rb ) {
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_left_symbol_match:
					
					if( shift_left() && *lb == op->Unit()->value ) {
						table[table_index] = lb;
						++table_index;
						COperationUnit::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_right_symbol_match:
					if( shift_right() && *rb == op->Unit()->value ) {
						table[table_index] = rb;
						++table_index;
						COperationUnit::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_left_parens_match:
					if( shift_left() && lb->IsLeftParen() ) {
						rb = lb->PairedParen();
						table[table_index] = lb;
						++table_index;
						table[table_index] = rb;
						++table_index;
						COperation::Next(op);
					} else {
						fail();
					}				
					break;

				case COperation::OT_right_parens_match:
					if( shift_right() && rb->IsRightParen() ) {
						lb = rb->PairedParen();
						table[table_index] = lb;
						++table_index;
						table[table_index] = rb;
						++table_index;
						COperation::Next(op);
					} else {
						fail();
					}				
					break;

				case COperation::OT_set_pointers:
					lb = table[op->IntInt()->x];
					rb = table[op->IntInt()->y];
					COperationIntInt::Next(op);
					break;

				case COperation::OT_left_s_variable_match:
					if( shift_left() && lb->IsSymbol() ) {
						table[table_index] = lb;
						++table_index;
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_right_s_variable_match:
					if( shift_right() && rb->IsSymbol() ) {
						table[table_index] = rb;
						++table_index;
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_left_duplicate_s_variable_match:
					if( shift_left() && *lb == *table[op->Int()->x] )
					{
						table[table_index] = lb;
						++table_index;
						COperationInt::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_right_duplicate_s_variable_match:
					if( shift_right() && *rb == *table[op->Int()->x] )
					{
						table[table_index] = rb;
						++table_index;
						COperationInt::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_closed_e_variable_match:
					table[table_index] = lb->Next();
					++table_index;
					table[table_index] = rb->Prev();
					++table_index;
					COperation::Next(op);
					break;

				case COperation::OT_left_duplicate_wve_variable_match:
					COperationInt::Next(op);
					break;

				case COperation::OT_right_duplicate_wve_variable_match:
					COperationInt::Next(op);
					break;

				case COperation::OT_left_w_variable_match:
					if( shift_left() ) {
						table[table_index] = lb;
						++table_index;
						if( lb->IsLeftParen() )	{
							lb = lb->PairedParen();
						}
						table[table_index] = lb;
						++table_index;
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_right_w_variable_match:
					if( shift_right() ) {
						table[table_index + 1] = rb;
						if( rb->IsRightParen() ) 	{
							rb = rb->PairedParen();
						}
						table[table_index] = rb;
						table_index += 2;
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_check_not_empty:
					if( table[table_index - 1]->Next() ==
						table[table_index - 2] ) {
						fail();
					} else {
						COperation::Next(op);
					}
					break;

				case COperation::OT_left_e_variable_match_begin:
					save();
					++stack_depth;
					table[table_index] = lb->Next();
					++table_index;
					table[table_index] = lb;
					++table_index;
					COperation::Next(op);
					if( op->Is(COperation::OT_left_ve_variable_match) ) {
						COperation::Next(op);
					} else {
						COperationQualifier::Next(op);
					}
					break;

				case COperation::OT_left_v_variable_match_begin:
					save();
					table[table_index] = lb->Next();
					table[table_index + 1] = lb;
					COperation::Next(op);
					break;

				case COperation::OT_left_ve_variable_match:
					lb = table[table_index + 1];
					if( shift_left() ) {
						if( lb->IsLeftParen() ) {
							lb = lb->PairedParen();
						}
						++stack_depth;
						table[table_index + 1] = lb;
						table_index += 2;
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_left_ve_variable_with_qualifier_match:
					lb = table[table_index + 1];
					if( shift_left() ) {
						if( lb->IsLeftParen() ) {
							lb = lb->PairedParen();
						}
						/* check qualifier */
						if( op->Qualifier()->qualifier->Check(lb) ) {
							++stack_depth;
							table[table_index + 1] = lb;
							table_index += 2;
							COperationQualifier::Next(op);
						} else {
							fail();
						}
					} else {
						fail();
					}
					break;

				case COperation::OT_right_e_variable_match_begin:
					save();
					++stack_depth;
					table[table_index] = rb;
					++table_index;
					table[table_index] = rb->Prev();
					++table_index;
					COperation::Next(op);
					if( op->Is(COperation::OT_right_ve_variable_match) ) {
						COperation::Next(op);
					} else {
						COperationQualifier::Next(op);
					}
					break;

				case COperation::OT_right_v_variable_match_begin:
					save();
					table[table_index] = rb;
					++table_index;
					table[table_index] = rb->Prev();
					++table_index;
					COperation::Next(op);
					break;

				case COperation::OT_right_ve_variable_match:
					rb = table[table_index];
					if( shift_right() ) {
						if( rb->IsRightParen() ) {
							rb = rb->PairedParen();
						}
						++stack_depth;
						table[table_index] = rb;
						table_index += 2;
						COperation::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_right_ve_variable_with_qualifier_match:
					rb = table[table_index];
					if( shift_right() ) {
						if( rb->IsRightParen() ) {
							rb = rb->PairedParen();
						}
						/* check qualifier */
						if( op->Qualifier()->qualifier->Check(rb) ) {
							++stack_depth;
							table[table_index] = rb;
							table_index += 2;
							COperationQualifier::Next(op);
						} else {
							fail();
						}
					} else {
						fail();
					}
					break;

				case COperation::OT_decriase_stack_depth:
					stack_depth -= operation->Int()->x;
					COperationInt::Next(op);
					break;

				case COperation::OT_check_qualifier_last_s_variable:
					if( op->Qualifier()->qualifier->
						Check(table[table_index - 1]) )
					{
						COperationQualifier::Next(op);
					} else {
						fail();
					}
					break;

				case COperation::OT_check_qualifier_last_wve_variable:
					{
						CUnitLink* i = table[table_index - 2]->Prev();
						CUnitLink* till = table[table_index - 1];
						while( i != till ) {
							i = i->Next();
							if( i->IsLeftParen() ) {
								i = i->PairedParen();
							}
							if( !( op->Qualifier()->qualifier->Check(i) ) ) {
								fail();
								break;
							}
						}
						if( i == till ) {
							COperationQualifier::Next(op);
						}
					}
					break;

				case COperation::OT_left_max_qualifier:
					lb = lb->Next();
					table[table_index] = lb;
					++table_index;
					for( ; lb != rb; lb = lb->Next() ) {
						if( !( op->Qualifier()->qualifier->Check(lb) ) ) {
							break;
						} else if( lb->IsLeftParen() ) {
							lb = lb->PairedParen();
						}
					}
					lb = lb->Prev();
					table[table_index] = lb;
					++table_index;
					COperationQualifier::Next(op);
					break;

				case COperation::OT_right_max_qualifier:
					rb = rb->Prev();
					table[table_index + 1] = lb;
					for( ; rb != lb; rb = rb->Prev() ) {
						if( !( op->Qualifier()->qualifier->Check(rb) ) ) {
							break;
						} else if( rb->IsRightParen() ) {
							rb = rb->PairedParen();
						}
					}
					rb = rb->Next();
					table[table_index] = rb;
					table_index += 2;
					COperationQualifier::Next(op);
					break;

				/* right part operations */
				case COperation::OT_insert_symbol:
					lb = CFieldOfView::Insert(lb, op->Unit()->value);
					COperationUnit::Next(op);
					break;

				case COperation::OT_insert_left_paren:
					{
						CUnitLink* paren = CFieldOfView::Insert(lb,
							CUnitValue(CLink::T_left_paren));
						lb = paren;
					}
					COperation::Next(op);
					break;

				case COperation::OT_insert_right_paren:
					{
						CUnitLink* paren = CFieldOfView::Insert(lb,
							CUnitValue(CLink::T_right_paren));
						lb = paren;
					}
					COperation::Next(op);
					break;

				case COperation::OT_insert_right_bracket:
					{
						CUnitLink* paren = CFieldOfView::Insert(lb,
							CUnitValue(CLink::T_right_paren));
						lb = paren;
					}
					COperation::Next(op);
					break;

				case COperation::OT_move_s:
					/* TODO: plan */
					COperationInt::Next(op);
					break;

				case COperation::OT_copy_s:
					lb =  CFieldOfView::Copy(lb, table[op->Int()->x]);
					COperationInt::Next(op);
					break;

				case COperation::OT_move_e:
					{
						int tmp = op->Int()->x;
						if( table[tmp]->Next() == table[tmp - 1] ) {
							break;
						}
					}
				case COperation::OT_move_wv:
					/* TODO: plan */
					COperationInt::Next(op);
					break;

				case COperation::OT_copy_wve:
					lb = CFieldOfView::Copy(lb, table[op->Int()->x - 1],
						table[op->Int()->x]);
					COperationInt::Next(op);
					break;

				case COperation::OT_return:
#if 0
					CFieldOfView::Remove(first, last);
					for( int i = 0; i < op->Int()->x; i += 2 ) {
						saved_points[i] =  saved_points[i]->Next();
					}
					for( int i = 0; i < op->Int()->x; i += 2 ) {
						CUnitLink* name = saved_points[i];
						if( name->IsLabel() ) {
							Run(name->Label()->second.operation, CFieldOfView(
								name, saved_points[i + 1]->Next()));
							CFieldOfView::Remove(name);
						} else {
							std::cout << "vse ochen' ploho" << i << "\n";
							view.Print();
							std::cout << "\n\n";
							/* TOWO: throw: trying to call not label */
						}
					}
#endif
					return;
					break;
				default:
					/* assert */
					break;
			}
		}
	} catch(...) {
		/* DEBUG INFO */
		throw;
	}
}

} // end of namespace refal2