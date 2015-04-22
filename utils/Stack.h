/*
 * Stack.h
 *
 *  Created on: 26.10.2014
 *      Author: helge
 */

#ifndef STACK_H_
#define STACK_H_


#include <Arduino.h>
#include "ListUtils.h"
#include "LinkedList.h"

template <class type> class Stack : public LinkedList<type> {

		void push(type item) {
			ListItem<type>* litem = new ListItem<type>(item);

			//relink
			if(this->head != NULL) {
				ListItem<type>* tmp = this->head;
				tmp->setPrevious(litem);
				litem->setNext(tmp);
			}

			this->head = litem;
		}

		type pop() {
			if(this->head != NULL) {
				ListItem<type>* tmp = this->head;

				//relink
				if(tmp != NULL) {
					this->head = tmp->getNext();
					if(this->head != NULL) {
						this->head->setPrevious(NULL);
					}
				}

				type item = tmp->getItem();
				delete tmp;

				return item;
			}

			return NULL;
		}

		type peek()  const {
			if(head != NULL) {
				return head->getItem();
			}
			return NULL;
		}
};



#endif /* STACK_H_ */
