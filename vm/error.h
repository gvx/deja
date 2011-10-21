#ifndef ERR_DEF
#define ERR_DEF

typedef enum
{
	Nothing,
	Exit,
	NameError,
	ValueError,
	TypeError,
	StackEmpty,
	IllegalFile,
	UserError
} Error;

#endif