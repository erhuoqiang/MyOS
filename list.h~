#define offsetof(TYPE, MEMBER)	((unsigned int)&(((TYPE *)0)->MEMBER))
#define list_entry(ptr, type, member)  ( (type *)(  ((char *)ptr) - offsetof(type, member) ) )
#define list_for_each(pos, head)	for( pos = head->next; pos != (head); pos = pos->next )
struct list_head
{
	struct list_head * next;
	struct list_head * prev;
};

extern inline void INIT_LIST_HEAD( struct list_head * list );
extern inline void __list_add( struct list_head * new_list, struct list_head * prev, struct list_head * next );
extern inline void list_add( struct list_head * new_list,  struct list_head * head );
extern inline void list_add_tail( struct list_head * new_list,  struct list_head * head );
extern inline void list_del( struct list_head * entry );
extern inline void list_remove_chain( struct list_head * begin, struct list_head * end );
extern inline void list_add_chain( struct list_head * begin, struct list_head * end, struct list_head * head );
extern inline void list_add_chain_tail( struct list_head *begin, struct list_head * end, struct list_head * head );
extern inline int list_empty(const struct list_head * head);
