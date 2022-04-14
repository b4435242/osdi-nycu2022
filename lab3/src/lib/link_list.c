#include "include/link_list.h"

Page* get_first_page(Page* head){
    return head->next;
}

Page* remove_page(Page* head, Page* page){ 
    if (head==NULL) 
        return NULL;
    if (head==page){
        Page* new_head = head->next;
        heap_free(head);
        head = new_head;
        return head;
    }
    Page *p = head;
    Page *n = head->next;
    while(p!=NULL&&n!=NULL){
        if (n==page){
            p->next = n->next;
            heap_free(page);
            break;
        }
        p = n;
        n = n->next;
    }
    return head;
}

Page* add_page(Page* head, Page* page){
    if (head==NULL){
        head = page;
    } else {
        Page* p = head->next;
        head->next = page;
        page->next = p;
    }
}

void add_page_with_index(Page* head, unsigned int index){
    Page* page = heap_malloc(sizeof(Page));
    page->next = NULL;
    page->index = index;
    add_page(head, page);
}

void remove_page_with_index(Page* head, unsigned int index){ // currently O(n) // skip list O(log n)
    Page* p = head->next;
    while(p!=NULL){
        if (p->index == index){
            remove_page(head, p);
            break;
        }
        p = p->next;
    }
}

Pool* add_pool(Pool* head, Pool* node){
    if (head==NULL){
        head = node;
    } else {
        Pool* p = head->next;
        head->next = node;
        node->next = p;
    }
}