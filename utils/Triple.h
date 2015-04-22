/*
 * Triple.h
 *
 *  Created on: 26.10.2014
 *      Author: helge
 */

#ifndef TRIPLE_H_
#define TRIPLE_H_

template<class type> class Triple {
	private:
		type a;
		type b;
		type c;

	public:
		Triple() {
			
		}
		
		Triple(type a, type b, type c) {
			this->a = a;
			this->b = b;
			this->c = c;
		}

		type getA() {
			return a;
		}

		type getB() {
			return b;
		}

		type getC() {
			return c;
		}
};

#endif /* TRIPLE_H_ */
