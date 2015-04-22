/*
 * SortedLinkedList.h
 *
 *  Created on: 27.10.2014
 *      Author: helge
 */

#ifndef SORTEDLINKEDLIST_H_
#define SORTEDLINKEDLIST_H_

#include <Arduino.h>
#include "LinkedList.h"

template<class type> class SortedLinkedList : public LinkedList<type> {
		SortedLinkedList() : LinkedList<type>() {

		}

		~SortedLinkedList() {
			
		}

		void insertSorted(type item) {
			ListItem<type>* item = new ListItem(item);
			
			ListItem<type>* tmp = cursor;

			//search with cursor
			cursorReset();
			while(item->getItem() >= getItemAtCursor() && cursor != NULL) {
				cursorNext();
			}
			this->addItemAtCursor(item);

			//reset cursor
			cursor = tmp;
		}
};


#endif /* SORTEDLINKEDLIST_H_ */
