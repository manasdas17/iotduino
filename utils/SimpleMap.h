/*
 * SimpleMap.h
 *
 * Created: 04.01.2015 02:39:56
 *  Author: helge
 */ 


#ifndef SIMPLEMAP_H_
#define SIMPLEMAP_H_

#include "LinkedList.h"

template<class K, class V>
class SimpleMap {
	private:
		LinkedList<K>* map_keys;
		LinkedList<V>* map_values;
	
	public:
		SimpleMap() {
			map_keys = new LinkedList<K>();
			map_values = new LinkedList<V>();
		}
	
		~SimpleMap() {
			delete map_keys;
			delete map_values;
		}
		
		uint16_t size() {
			return map_keys->size();
		}
		
		boolean isEmpty() {
			return size() == 0;
		}
		
		V get(K key) {
			goToKey(key);
			return map_values->getItemAtCursor();
		}
		
		boolean containsKey(K key) {
			goToKey(key);
			return key == map_keys->getItemAtCursor();
		}
		
		boolean containsValue(V val) {
			map_values->cursorReset();
			while(val != map_values->getItemAtCursor()) {
				map_values->cursorNext();
			}
			
			return val == map_values->getItemAtCursor();
		}
		
		V remove(K key) {
			goToKey(key);
			V tmp = map_values->getItemAtCursor();
			map_keys->deleteItemAtCursor();
			map_values->deleteItemAtCursor();
			return tmp;			
		}
		
		V put(K key, V value) {
			V tmp = remove(key);
			map_keys->cursorReset();
			map_values->cursorReset();
			map_keys->addItemAtCursor(key);
			map_values->addItemAtCursor(value);
			return tmp;
		}
		
		void clear() {
			map_keys->clear();
			map_values->clear();
		}
		
		LinkedList<V>* values() {
			return map_values;
		}

		LinkedList<K>* keys() {
			return map_keys;
		}

	private:
		void goToKey(K key) {
			map_keys->cursorReset();
			map_values->cursorReset();
			
			while(key != map_keys->getItemAtCursor()) {
				map_keys->cursorNext();
				map_values->cursorNext();
			}
		}
};



#endif /* SIMPLEMAP_H_ */