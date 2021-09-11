/*
 * Compile-time type checking which is originally seen in linux kernel for module's
 * variables used during initialization for its argument type check.
 *
 * These macros are intended to be used in global scope, not inside another function
 * or else compiler would error out "error: invalid storage class for function ...".
 *
 * This would shows mismatched type when compile the program, if any.
 */

#define type_check(name, var, type) \
	type_check_detail(name, &(var), type)

#define type_check_detail(name, var, type) \
	static inline type __attribute__((__unused__)) *__chck_##name(void) { return (var); }

int local_var;
type_check(MyVarname, local_var, int);         // match
type_check(MyVarName2, local_var, double);     // mis-match

int main(void)
{
	/* really nothing here */
	return 0;
}
