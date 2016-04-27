
struct list_head
{
	struct list_head * next;
	struct list_head * prev;
};

inline void INIT_LIST_HEAD( struct list_head * list )
{
	list->next = list;
	list->prev = list;
}

inline void __list_add( struct list_head * new_list, struct list_head * prev, struct list_head * next )
{
	next->prev = new_list;
	new_list->next = next;
	new_list->prev = prev;
	prev->next = new_list;
}

inline void list_add( struct list_head * new_list,  struct list_head * head )
{
	 __list_add(new_list, head, head->next);
}

inline void list_add_tail( struct list_head * new_list,  struct list_head * head )
{
	 __list_add(new_list, head->prev, head);
	
}

inline void __list_del( struct list_head * pre,  struct list_head * next )
{
	 pre->next = next;
	 next->prev = pre;
}

inline void list_del( struct list_head * entry )
{
	__list_del( entry->prev, entry->next);
}

inline void list_remove_chain( struct list_head * begin, struct list_head * end )
{
	begin->prev->next = end->next;
	end->next->prev = begin->prev;
}

inline void list_add_chain( struct list_head * begin, struct list_head * end, struct list_head * head )
{
	begin->prev = head;
	end->next = head->next;
	head->next->prev = end;
	head->next = begin;
}

inline void list_add_chain_tail( struct list_head *begin, struct list_head * end, struct list_head * head )
{
	begin->prev = head->prev;
	end->next = head;
	head->prev->next = begin;
	head->prev = end;
}

inline int list_empty(const struct list_head * head)
{
	return head->next == head;
}


	
