/*
 * Queue
 *
 *  Created on: 26.10.2014
 *      Author: helge
 */

#ifndef QUEUE_
#define QUEUE_

#include <Arduino.h>
#include "LinkedList.h"

template <class type> class Queue : public LinkedList<type> {

	public:
		void push(type* item) {
			ListItem<type>* litem = new ListItem<type>(item);

			//relink
			if(this->head != NULL) {
				ListItem<type>* tmp = this->head;
				tmp->setPrevious(litem);
				litem->setNext(tmp);
			} else {
				this->tail = litem;
			}

			this->head = litem;
		}

		boolean pop(type* t) {
			if(this->tail != NULL) {
				ListItem<type>* tmp = this->tail;
				ListItem<type>* prev = tmp->getPrevious();

				//relink
				if(prev != NULL) {
					prev->setNext(NULL);
				}
				this->tail = prev;

				if(this->head == tmp) {
					this->head = prev;
				}

				memcpy(t, tmp->getItem(), sizeof(type));
				delete tmp;

				return true;
			}

			return false;
		}

		boolean peek(type* t) const {
			if(this->tail != NULL) {
				memcpy(t, this->tail->getItem(), sizeof(type));
				return true;
			}
			return false;
		}
};


#endif /* QUEUE_ */
