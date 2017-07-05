

struct lock_functions
{
	int (*lock)(void*);
	int (*unlock)(void*);
	int (*trylock)(void*);
	int (*destroy)(void*);
};

struct long_lock
{

}

