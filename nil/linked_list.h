#pragma once

#define linked_list_s(T) struct linked_list$$##T
#define linked_list(T) linked_list_s(T) { linked_list_s(T)* next; T data; }

/*typedef struct linked_list {
	struct linked_list* next;
} linked_list;*/